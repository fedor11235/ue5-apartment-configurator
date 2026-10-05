#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Data/ConfiguratorTypes.h"
#include "ConfiguratorPlayerController.generated.h"

class AConfiguratorCameraController;
class AApartmentActor;
class UConfiguratorHUDWidget;
class UBuildingConfigSubsystem;

/**
 * Orchestrates the whole experience ("director"):
 *   - kicks off async config loading;
 *   - on load, binds the in-level AApartmentActors to their data, builds the floor panel
 *     and takes control of the ConfiguratorCameraController;
 *   - routes UI events (floor selected, apartment clicked, hide-sold toggled, back) to the
 *     camera and the apartment actors.
 *
 * Interaction, camera and UI are otherwise decoupled — they only know their own job and
 * raise delegates; this controller is the single place that wires them together.
 */
UCLASS()
class APARTMENTCONFIGURATOR_API AConfiguratorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AConfiguratorPlayerController();

	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Root HUD widget class, set on the Blueprint controller. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator|UI")
	TSubclassOf<UConfiguratorHUDWidget> HUDWidgetClass;

private:
	UFUNCTION()
	void HandleConfigLoaded(bool bSuccess, const FBuildingConfig& Config);

	UFUNCTION()
	void HandleFloorSelected(int32 FloorNumber);

	UFUNCTION()
	void HandleHideSoldChanged(bool bHideSold);

	UFUNCTION()
	void HandleApartmentClicked(AApartmentActor* Apartment);

	UFUNCTION()
	void HandleBackRequested();

	/** Match the in-level apartment actors to parsed data and subscribe to their clicks. */
	void BindApartmentActors();

	/** Focus an apartment: move the camera and open its card. */
	void FocusApartment(const FApartmentData& Data, AApartmentActor* Actor);

	void SyncBackButton();

	UBuildingConfigSubsystem* GetConfigSubsystem() const;

	UPROPERTY()
	TObjectPtr<AConfiguratorCameraController> CameraController;

	UPROPERTY()
	TObjectPtr<UConfiguratorHUDWidget> HUDWidget;

	/** In-level apartment actors keyed by id (bound on load). */
	UPROPERTY()
	TMap<FString, TObjectPtr<AApartmentActor>> ApartmentActors;

	UPROPERTY()
	TObjectPtr<AApartmentActor> HighlightedActor;

	bool bDragOrbiting = false;
};
