#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/ConfiguratorTypes.h"
#include "ConfiguratorCameraController.generated.h"

class UCameraComponent;
class USceneComponent;

/** The three navigation modes required by the brief. */
UENUM(BlueprintType)
enum class EConfiguratorCameraMode : uint8
{
	/** Whole building, orbiting freely around it. */
	Genplan		UMETA(DisplayName = "Genplan"),
	/** Centred on a single floor (vertical move). */
	Floor		UMETA(DisplayName = "Floor"),
	/** Close-up on one apartment, using coordinates from JSON. */
	Apartment	UMETA(DisplayName = "Apartment")
};

/** A single camera destination plus the mode it represents (one entry on the back stack). */
USTRUCT()
struct FCameraViewState
{
	GENERATED_BODY()

	UPROPERTY() EConfiguratorCameraMode Mode = EConfiguratorCameraMode::Genplan;
	UPROPERTY() FVector Location = FVector::ZeroVector;
	UPROPERTY() FRotator Rotation = FRotator::ZeroRotator;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraModeChanged, EConfiguratorCameraMode, NewMode);

/**
 * Drives the view camera. Interpolates smoothly from the current transform to the active
 * target every tick, keeps a history stack so GoBack() returns to the previous view, and
 * — in Genplan mode — orbits the building from yaw input.
 *
 * Placed in the level and possessed-by/viewed-through the player controller (SetViewTargetWithBlend
 * is intentionally avoided so blend behaviour stays fully under our control).
 */
UCLASS()
class APARTMENTCONFIGURATOR_API AConfiguratorCameraController : public AActor
{
	GENERATED_BODY()

public:
	AConfiguratorCameraController();

	virtual void Tick(float DeltaSeconds) override;

	/** Orbit the building. Pushes a Genplan view and clears the back stack (it is the root view). */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Camera")
	void FocusGenplan();

	/** Move to the floor's focus location, looking at the building. Pushes onto the back stack. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Camera")
	void FocusFloor(const FFloorData& Floor);

	/** Move to the apartment's JSON focus transform. Pushes onto the back stack. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Camera")
	void FocusApartment(const FApartmentData& Apartment);

	/** Return to the previous view. No-op (returns false) when already at the root Genplan view. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Camera")
	bool GoBack();

	UFUNCTION(BlueprintCallable, Category = "Configurator|Camera")
	bool CanGoBack() const { return ViewStack.Num() > 1; }

	UFUNCTION(BlueprintCallable, Category = "Configurator|Camera")
	EConfiguratorCameraMode GetCurrentMode() const { return CurrentMode; }

	/** Feed orbit input (e.g. from mouse drag) — only has an effect in Genplan mode. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Camera")
	void AddOrbitInput(float YawDelta);

	UPROPERTY(BlueprintAssignable, Category = "Configurator|Camera")
	FOnCameraModeChanged OnCameraModeChanged;

protected:
	virtual void BeginPlay() override;

	/** World-space point the camera orbits / looks toward (centre of the building). */
	UPROPERTY(EditAnywhere, Category = "Configurator|Camera")
	FVector BuildingCenter = FVector(0.f, 0.f, 500.f);

	/** Orbit radius and height used to derive the Genplan transform. */
	UPROPERTY(EditAnywhere, Category = "Configurator|Camera")
	float GenplanRadius = 1800.f;

	UPROPERTY(EditAnywhere, Category = "Configurator|Camera")
	float GenplanHeight = 900.f;

	/** Higher = snappier transitions. Interpolation is frame-rate independent. */
	UPROPERTY(EditAnywhere, Category = "Configurator|Camera", meta = (ClampMin = "0.5"))
	float InterpSpeed = 4.f;

	UPROPERTY(EditAnywhere, Category = "Configurator|Camera")
	float OrbitSensitivity = 0.5f;

private:
	UPROPERTY(VisibleAnywhere, Category = "Configurator|Camera")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Configurator|Camera")
	TObjectPtr<UCameraComponent> Camera;

	/** Push a view, make it the interpolation target and announce the mode change. */
	void PushView(const FCameraViewState& View);

	/** Compute the orbiting Genplan transform for the current orbit yaw. */
	FCameraViewState MakeGenplanView() const;

	UPROPERTY()
	TArray<FCameraViewState> ViewStack;

	EConfiguratorCameraMode CurrentMode = EConfiguratorCameraMode::Genplan;

	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	float OrbitYaw = 0.f;
};
