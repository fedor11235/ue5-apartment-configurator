#include "ConfiguratorPlayerController.h"
#include "ApartmentConfigurator.h"

#include "Camera/ConfiguratorCameraController.h"
#include "Data/BuildingConfigSubsystem.h"
#include "Interaction/ApartmentActor.h"
#include "UI/ApartmentCardWidget.h"
#include "UI/ConfiguratorHUDWidget.h"
#include "UI/FloorPanelWidget.h"

#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

AConfiguratorPlayerController::AConfiguratorPlayerController()
{
	// The configurator is a cursor-driven UI experience.
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AConfiguratorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Take control of the level's camera controller and view through it.
	CameraController = Cast<AConfiguratorCameraController>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AConfiguratorCameraController::StaticClass()));
	if (CameraController)
	{
		SetViewTarget(CameraController);
	}
	else
	{
		UE_LOG(LogConfigurator, Error, TEXT("No ConfiguratorCameraController found in the level."));
	}

	// Build the HUD and subscribe to its signals.
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UConfiguratorHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			HUDWidget->OnBackRequested.AddDynamic(this, &AConfiguratorPlayerController::HandleBackRequested);

			if (UFloorPanelWidget* Panel = HUDWidget->GetFloorPanel())
			{
				Panel->OnFloorSelected.AddDynamic(this, &AConfiguratorPlayerController::HandleFloorSelected);
				Panel->OnHideSoldChanged.AddDynamic(this, &AConfiguratorPlayerController::HandleHideSoldChanged);
			}
		}
	}
	else
	{
		UE_LOG(LogConfigurator, Error, TEXT("HUDWidgetClass is not set on the player controller."));
	}

	// Load configuration asynchronously; everything else wires up in HandleConfigLoaded.
	if (UBuildingConfigSubsystem* Sub = GetConfigSubsystem())
	{
		Sub->OnConfigLoaded.AddDynamic(this, &AConfiguratorPlayerController::HandleConfigLoaded);
		Sub->LoadConfigAsync();
	}
}

void AConfiguratorPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UBuildingConfigSubsystem* Sub = GetConfigSubsystem())
	{
		Sub->OnConfigLoaded.RemoveDynamic(this, &AConfiguratorPlayerController::HandleConfigLoaded);
	}
	Super::EndPlay(EndPlayReason);
}

void AConfiguratorPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// Hold left mouse to orbit in Genplan mode.
	bDragOrbiting = IsInputKeyDown(EKeys::LeftMouseButton);
	if (bDragOrbiting && CameraController)
	{
		float MouseDX = 0.f, MouseDY = 0.f;
		GetInputMouseDelta(MouseDX, MouseDY);
		CameraController->AddOrbitInput(MouseDX);
	}
}

void AConfiguratorPlayerController::HandleConfigLoaded(bool bSuccess, const FBuildingConfig& Config)
{
	if (!bSuccess)
	{
		// Robustness: a failed load must not crash — the scene simply stays at Genplan with no buttons.
		UE_LOG(LogConfigurator, Error, TEXT("Configuration failed to load; UI left empty."));
		return;
	}

	BindApartmentActors();

	if (HUDWidget)
	{
		if (UFloorPanelWidget* Panel = HUDWidget->GetFloorPanel())
		{
			Panel->BuildFromConfig(Config);
		}
	}
	SyncBackButton();
}

void AConfiguratorPlayerController::BindApartmentActors()
{
	ApartmentActors.Reset();

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AApartmentActor::StaticClass(), Found);

	UBuildingConfigSubsystem* Sub = GetConfigSubsystem();
	for (AActor* Actor : Found)
	{
		AApartmentActor* Apt = Cast<AApartmentActor>(Actor);
		if (!Apt)
		{
			continue;
		}

		// The placed actor already carries an id (set in the Blueprint instance); bind its data.
		const FApartmentData* Data = Sub ? Sub->FindApartment(Apt->GetApartmentId()) : nullptr;
		if (!Data)
		{
			UE_LOG(LogConfigurator, Warning,
				TEXT("Apartment actor '%s' has id '%s' with no matching config entry."),
				*Apt->GetName(), *Apt->GetApartmentId());
			continue;
		}

		Apt->InitializeFromData(*Data);
		Apt->OnApartmentClicked.AddDynamic(this, &AConfiguratorPlayerController::HandleApartmentClicked);
		ApartmentActors.Add(Data->Id, Apt);
	}

	UE_LOG(LogConfigurator, Log, TEXT("Bound %d apartment actor(s)."), ApartmentActors.Num());
}

void AConfiguratorPlayerController::HandleFloorSelected(int32 FloorNumber)
{
	UBuildingConfigSubsystem* Sub = GetConfigSubsystem();
	const FFloorData* Floor = Sub ? Sub->FindFloor(FloorNumber) : nullptr;
	if (Floor && CameraController)
	{
		CameraController->FocusFloor(*Floor);
	}
	SyncBackButton();
}

void AConfiguratorPlayerController::HandleApartmentClicked(AApartmentActor* Apartment)
{
	if (Apartment)
	{
		FocusApartment(Apartment->GetData(), Apartment);
	}
}

void AConfiguratorPlayerController::FocusApartment(const FApartmentData& Data, AApartmentActor* Actor)
{
	if (CameraController)
	{
		CameraController->FocusApartment(Data);
	}

	// Move the highlight to the focused actor.
	if (HighlightedActor && HighlightedActor != Actor)
	{
		HighlightedActor->SetHighlighted(false);
	}
	HighlightedActor = Actor;
	if (HighlightedActor)
	{
		HighlightedActor->SetHighlighted(true);
	}

	if (HUDWidget)
	{
		if (UApartmentCardWidget* Card = HUDWidget->GetApartmentCard())
		{
			Card->ShowApartment(Data);
		}
	}
	SyncBackButton();
}

void AConfiguratorPlayerController::HandleHideSoldChanged(bool bHideSold)
{
	// Visually differentiate + disable sold units in 3D.
	for (const TPair<FString, TObjectPtr<AApartmentActor>>& Pair : ApartmentActors)
	{
		AApartmentActor* Apt = Pair.Value;
		if (Apt && Apt->GetData().IsSold())
		{
			Apt->SetFilteredOut(bHideSold);
		}
	}
}

void AConfiguratorPlayerController::HandleBackRequested()
{
	if (!CameraController)
	{
		return;
	}

	// Leaving apartment focus clears the card + highlight.
	if (CameraController->GetCurrentMode() == EConfiguratorCameraMode::Apartment)
	{
		if (HUDWidget)
		{
			if (UApartmentCardWidget* Card = HUDWidget->GetApartmentCard())
			{
				Card->Hide();
			}
		}
		if (HighlightedActor)
		{
			HighlightedActor->SetHighlighted(false);
			HighlightedActor = nullptr;
		}
	}

	CameraController->GoBack();
	SyncBackButton();
}

void AConfiguratorPlayerController::SyncBackButton()
{
	if (HUDWidget && CameraController)
	{
		HUDWidget->SetBackEnabled(CameraController->CanGoBack());
	}
}

UBuildingConfigSubsystem* AConfiguratorPlayerController::GetConfigSubsystem() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UBuildingConfigSubsystem>() : nullptr;
}
