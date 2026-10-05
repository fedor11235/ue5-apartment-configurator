#include "BuildingConfigSubsystem.h"
#include "ApartmentConfigurator.h"

#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UBuildingConfigSubsystem::LoadConfigAsync(const FString& RelativeOrAbsolutePath)
{
	const FString Path = ResolvePath(RelativeOrAbsolutePath);
	UE_LOG(LogConfigurator, Log, TEXT("Loading building config from: %s"), *Path);

	// Keep a weak handle so a destroyed subsystem (e.g. PIE stopped mid-load) won't be touched.
	TWeakObjectPtr<UBuildingConfigSubsystem> WeakThis(this);

	// File read + parse on a background thread so the game thread never blocks on IO.
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis, Path]()
	{
		FBuildingConfig Parsed;
		bool bSuccess = false;

		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *Path))
		{
			UE_LOG(LogConfigurator, Error, TEXT("Config file not found or unreadable: %s"), *Path);
		}
		else
		{
			bSuccess = ParseConfig(JsonText, Parsed);
		}

		// Hop back to the game thread before touching the UObject or firing the delegate.
		AsyncTask(ENamedThreads::GameThread, [WeakThis, bSuccess, Parsed]()
		{
			if (UBuildingConfigSubsystem* Strong = WeakThis.Get())
			{
				Strong->HandleLoadComplete(bSuccess, Parsed);
			}
		});
	});
}

void UBuildingConfigSubsystem::HandleLoadComplete(bool bSuccess, const FBuildingConfig& Parsed)
{
	// A parse that technically succeeded but yielded zero floors is treated as a failure.
	const bool bUsable = bSuccess && Parsed.IsValid();
	if (bUsable)
	{
		CachedConfig = Parsed;
		bLoaded = true;
		UE_LOG(LogConfigurator, Log, TEXT("Config loaded: '%s', %d floor(s)."),
			*CachedConfig.BuildingName, CachedConfig.Floors.Num());
	}
	else
	{
		UE_LOG(LogConfigurator, Error, TEXT("Config load failed or empty — UI will show an error state."));
	}

	OnConfigLoaded.Broadcast(bUsable, CachedConfig);
}

FString UBuildingConfigSubsystem::ResolvePath(const FString& InPath)
{
	if (InPath.IsEmpty())
	{
		return FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("BuildingConfig.json"));
	}
	if (FPaths::IsRelative(InPath))
	{
		return FPaths::Combine(FPaths::ProjectDir(), InPath);
	}
	return InPath;
}

bool UBuildingConfigSubsystem::ParseConfig(const FString& JsonText, FBuildingConfig& OutConfig)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogConfigurator, Error, TEXT("JSON is malformed and could not be deserialized."));
		return false;
	}

	// buildingName is optional — default to a placeholder rather than failing.
	if (!Root->TryGetStringField(TEXT("buildingName"), OutConfig.BuildingName) || OutConfig.BuildingName.IsEmpty())
	{
		OutConfig.BuildingName = TEXT("Unnamed Building");
	}

	const TArray<TSharedPtr<FJsonValue>>* FloorsArray = nullptr;
	if (!Root->TryGetArrayField(TEXT("floors"), FloorsArray) || !FloorsArray)
	{
		UE_LOG(LogConfigurator, Error, TEXT("Missing or invalid 'floors' array."));
		return false;
	}

	for (const TSharedPtr<FJsonValue>& FloorValue : *FloorsArray)
	{
		const TSharedPtr<FJsonObject>* FloorObjPtr = nullptr;
		if (!FloorValue.IsValid() || !FloorValue->TryGetObject(FloorObjPtr) || !FloorObjPtr)
		{
			UE_LOG(LogConfigurator, Warning, TEXT("Skipping a floor entry that is not an object."));
			continue;
		}

		FFloorData Floor;
		if (ParseFloor(*FloorObjPtr, Floor))
		{
			OutConfig.Floors.Add(MoveTemp(Floor));
		}
	}

	return true;
}

bool UBuildingConfigSubsystem::ParseFloor(const TSharedPtr<FJsonObject>& FloorObj, FFloorData& OutFloor)
{
	if (!FloorObj.IsValid())
	{
		return false;
	}

	// Absent/invalid fields degrade to defaults instead of erroring.
	FloorObj->TryGetNumberField(TEXT("floorNumber"), OutFloor.FloorNumber);
	if (!FloorObj->TryGetStringField(TEXT("displayName"), OutFloor.DisplayName) || OutFloor.DisplayName.IsEmpty())
	{
		OutFloor.DisplayName = FString::Printf(TEXT("Floor %d"), OutFloor.FloorNumber);
	}

	const TSharedPtr<FJsonObject>* FocusObj = nullptr;
	if (FloorObj->TryGetObjectField(TEXT("focusLocation"), FocusObj) && FocusObj)
	{
		OutFloor.FocusLocation = ParseVector(*FocusObj, FVector::ZeroVector);
	}

	const TArray<TSharedPtr<FJsonValue>>* AptArray = nullptr;
	if (FloorObj->TryGetArrayField(TEXT("apartments"), AptArray) && AptArray)
	{
		for (const TSharedPtr<FJsonValue>& AptValue : *AptArray)
		{
			const TSharedPtr<FJsonObject>* AptObjPtr = nullptr;
			if (!AptValue.IsValid() || !AptValue->TryGetObject(AptObjPtr) || !AptObjPtr)
			{
				UE_LOG(LogConfigurator, Warning, TEXT("Skipping an apartment entry that is not an object."));
				continue;
			}

			FApartmentData Apt;
			if (ParseApartment(*AptObjPtr, Apt))
			{
				OutFloor.Apartments.Add(MoveTemp(Apt));
			}
		}
	}

	return true;
}

bool UBuildingConfigSubsystem::ParseApartment(const TSharedPtr<FJsonObject>& AptObj, FApartmentData& OutApt)
{
	if (!AptObj.IsValid())
	{
		return false;
	}

	if (!AptObj->TryGetStringField(TEXT("id"), OutApt.Id) || OutApt.Id.IsEmpty())
	{
		// An apartment with no id cannot be referenced by the UI — drop it with a warning.
		UE_LOG(LogConfigurator, Warning, TEXT("Dropping apartment without an 'id'."));
		return false;
	}

	FString StatusRaw;
	AptObj->TryGetStringField(TEXT("status"), StatusRaw);
	OutApt.Status = ParseStatus(StatusRaw);

	double AreaValue = 0.0;
	if (AptObj->TryGetNumberField(TEXT("area"), AreaValue))
	{
		OutApt.Area = static_cast<float>(AreaValue);
	}

	const TSharedPtr<FJsonObject>* LocObj = nullptr;
	if (AptObj->TryGetObjectField(TEXT("focusLocation"), LocObj) && LocObj)
	{
		OutApt.FocusLocation = ParseVector(*LocObj, FVector::ZeroVector);
	}

	const TSharedPtr<FJsonObject>* RotObj = nullptr;
	if (AptObj->TryGetObjectField(TEXT("focusRotation"), RotObj) && RotObj)
	{
		OutApt.FocusRotation = ParseRotator(*RotObj, FRotator::ZeroRotator);
	}

	return true;
}

FVector UBuildingConfigSubsystem::ParseVector(const TSharedPtr<FJsonObject>& Obj, const FVector& Fallback)
{
	FVector Out = Fallback;
	if (Obj.IsValid())
	{
		double V = 0.0;
		if (Obj->TryGetNumberField(TEXT("x"), V)) { Out.X = V; }
		if (Obj->TryGetNumberField(TEXT("y"), V)) { Out.Y = V; }
		if (Obj->TryGetNumberField(TEXT("z"), V)) { Out.Z = V; }
	}
	return Out;
}

FRotator UBuildingConfigSubsystem::ParseRotator(const TSharedPtr<FJsonObject>& Obj, const FRotator& Fallback)
{
	FRotator Out = Fallback;
	if (Obj.IsValid())
	{
		double V = 0.0;
		if (Obj->TryGetNumberField(TEXT("pitch"), V)) { Out.Pitch = V; }
		if (Obj->TryGetNumberField(TEXT("yaw"),   V)) { Out.Yaw = V; }
		if (Obj->TryGetNumberField(TEXT("roll"),  V)) { Out.Roll = V; }
	}
	return Out;
}

EApartmentStatus UBuildingConfigSubsystem::ParseStatus(const FString& Raw)
{
	// Case-insensitive; anything that isn't explicitly "sold" is treated as available.
	if (Raw.Equals(TEXT("Sold"), ESearchCase::IgnoreCase))
	{
		return EApartmentStatus::Sold;
	}
	if (!Raw.Equals(TEXT("Available"), ESearchCase::IgnoreCase) && !Raw.IsEmpty())
	{
		UE_LOG(LogConfigurator, Warning, TEXT("Unknown status '%s' — defaulting to Available."), *Raw);
	}
	return EApartmentStatus::Available;
}

const FFloorData* UBuildingConfigSubsystem::FindFloor(int32 FloorNumber) const
{
	return CachedConfig.Floors.FindByPredicate(
		[FloorNumber](const FFloorData& F) { return F.FloorNumber == FloorNumber; });
}

const FApartmentData* UBuildingConfigSubsystem::FindApartment(const FString& ApartmentId) const
{
	for (const FFloorData& Floor : CachedConfig.Floors)
	{
		if (const FApartmentData* Found = Floor.Apartments.FindByPredicate(
			[&ApartmentId](const FApartmentData& A) { return A.Id == ApartmentId; }))
		{
			return Found;
		}
	}
	return nullptr;
}
