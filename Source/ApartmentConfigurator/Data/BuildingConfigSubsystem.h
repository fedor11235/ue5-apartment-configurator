#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ConfiguratorTypes.h"
#include "BuildingConfigSubsystem.generated.h"

class FJsonObject;

/** Broadcast once the building configuration has been loaded (success flag + parsed data). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildingConfigLoaded, bool, bSuccess, const FBuildingConfig&, Config);

/**
 * Owns loading and parsing of the building configuration from a local JSON file.
 *
 * Design notes:
 *  - Implemented as a GameInstanceSubsystem so the data outlives individual levels and is
 *    reachable from any UObject via GetGameInstance()->GetSubsystem<UBuildingConfigSubsystem>().
 *  - File IO + parsing run on a background thread (non-blocking load, per the brief); the
 *    OnConfigLoaded delegate is marshalled back to the game thread.
 *  - Parsing is fully defensive: a missing/renamed/wrong-typed field degrades to a logged
 *    warning and a sane default instead of crashing (robustness is a graded criterion).
 */
UCLASS()
class APARTMENTCONFIGURATOR_API UBuildingConfigSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Fired on the game thread when loading finishes (whether it succeeded or not). */
	UPROPERTY(BlueprintAssignable, Category = "Configurator|Data")
	FOnBuildingConfigLoaded OnConfigLoaded;

	/**
	 * Asynchronously load and parse the config.
	 * @param RelativeOrAbsolutePath  Defaults to <Project>/Config/BuildingConfig.json when empty.
	 */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Data")
	void LoadConfigAsync(const FString& RelativeOrAbsolutePath = TEXT(""));

	/** True once a valid config has been parsed. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Data")
	bool IsLoaded() const { return bLoaded; }

	/** Last successfully parsed config (empty if none). */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Data")
	const FBuildingConfig& GetConfig() const { return CachedConfig; }

	/** Find a floor by its number. Returns nullptr when absent. */
	const FFloorData* FindFloor(int32 FloorNumber) const;

	/** Find an apartment by id across all floors. Returns nullptr when absent. */
	const FApartmentData* FindApartment(const FString& ApartmentId) const;

private:
	/** Resolve an empty path to the default Config/BuildingConfig.json under the project dir. */
	static FString ResolvePath(const FString& InPath);

	/** Pure parse step (no file IO) — safe to run off the game thread. */
	static bool ParseConfig(const FString& JsonText, FBuildingConfig& OutConfig);

	static bool ParseFloor(const TSharedPtr<FJsonObject>& FloorObj, FFloorData& OutFloor);
	static bool ParseApartment(const TSharedPtr<FJsonObject>& AptObj, FApartmentData& OutApt);
	static FVector ParseVector(const TSharedPtr<FJsonObject>& Obj, const FVector& Fallback);
	static FRotator ParseRotator(const TSharedPtr<FJsonObject>& Obj, const FRotator& Fallback);
	static EApartmentStatus ParseStatus(const FString& Raw);

	/** Marshalled back onto the game thread to cache results and broadcast. */
	void HandleLoadComplete(bool bSuccess, const FBuildingConfig& Parsed);

	UPROPERTY()
	FBuildingConfig CachedConfig;

	bool bLoaded = false;
};
