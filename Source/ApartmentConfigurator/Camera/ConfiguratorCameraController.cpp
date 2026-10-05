#include "ConfiguratorCameraController.h"
#include "ApartmentConfigurator.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetMathLibrary.h"

AConfiguratorCameraController::AConfiguratorCameraController()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
}

void AConfiguratorCameraController::BeginPlay()
{
	Super::BeginPlay();

	// Start from the Genplan overview. Seed the live transform so we don't lerp from the origin.
	FocusGenplan();
	const FCameraViewState Start = MakeGenplanView();
	SetActorLocationAndRotation(Start.Location, Start.Rotation);
}

void AConfiguratorCameraController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// In Genplan the target continuously tracks the orbit angle so drag input feels live.
	if (CurrentMode == EConfiguratorCameraMode::Genplan)
	{
		const FCameraViewState Orbit = MakeGenplanView();
		TargetLocation = Orbit.Location;
		TargetRotation = Orbit.Rotation;
	}

	// Frame-rate-independent smoothing toward the active target.
	const FVector NewLoc = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaSeconds, InterpSpeed);
	const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaSeconds, InterpSpeed);
	SetActorLocationAndRotation(NewLoc, NewRot);
}

void AConfiguratorCameraController::FocusGenplan()
{
	ViewStack.Reset();
	const FCameraViewState View = MakeGenplanView();
	PushView(View);
}

void AConfiguratorCameraController::FocusFloor(const FFloorData& Floor)
{
	FCameraViewState View;
	View.Mode = EConfiguratorCameraMode::Floor;
	View.Location = Floor.FocusLocation;
	// Look from the floor's focus point toward the building centre.
	View.Rotation = UKismetMathLibrary::FindLookAtRotation(Floor.FocusLocation, BuildingCenter);
	PushView(View);
}

void AConfiguratorCameraController::FocusApartment(const FApartmentData& Apartment)
{
	FCameraViewState View;
	View.Mode = EConfiguratorCameraMode::Apartment;
	View.Location = Apartment.FocusLocation;
	View.Rotation = Apartment.FocusRotation;
	PushView(View);
}

bool AConfiguratorCameraController::GoBack()
{
	if (!CanGoBack())
	{
		return false;
	}

	ViewStack.Pop();
	const FCameraViewState& Prev = ViewStack.Last();

	CurrentMode = Prev.Mode;
	TargetLocation = Prev.Location;
	TargetRotation = Prev.Rotation;

	// When returning to Genplan, resync the orbit angle to the stored view so it doesn't jump.
	if (Prev.Mode == EConfiguratorCameraMode::Genplan)
	{
		OrbitYaw = (Prev.Location - BuildingCenter).Rotation().Yaw;
	}

	OnCameraModeChanged.Broadcast(CurrentMode);
	return true;
}

void AConfiguratorCameraController::AddOrbitInput(float YawDelta)
{
	if (CurrentMode == EConfiguratorCameraMode::Genplan)
	{
		OrbitYaw += YawDelta * OrbitSensitivity;
	}
}

void AConfiguratorCameraController::PushView(const FCameraViewState& View)
{
	ViewStack.Add(View);
	CurrentMode = View.Mode;
	TargetLocation = View.Location;
	TargetRotation = View.Rotation;
	OnCameraModeChanged.Broadcast(CurrentMode);
}

FCameraViewState AConfiguratorCameraController::MakeGenplanView() const
{
	// Position on a circle of radius GenplanRadius around the building, at GenplanHeight.
	const float Rad = FMath::DegreesToRadians(OrbitYaw);
	const FVector Offset(FMath::Cos(Rad) * GenplanRadius, FMath::Sin(Rad) * GenplanRadius, GenplanHeight);

	FCameraViewState View;
	View.Mode = EConfiguratorCameraMode::Genplan;
	View.Location = BuildingCenter + Offset;
	View.Rotation = UKismetMathLibrary::FindLookAtRotation(View.Location, BuildingCenter);
	return View;
}
