#pragma once

#include "CoreMinimal.h"
#include "ConfiguratorTypes.generated.h"

/** Sale status of a single apartment. */
UENUM(BlueprintType)
enum class EApartmentStatus : uint8
{
	Available	UMETA(DisplayName = "Available"),
	Sold		UMETA(DisplayName = "Sold")
};

/**
 * One apartment. Mirrors a single object inside a floor's "apartments" array in
 * BuildingConfig.json. FocusLocation / FocusRotation drive the Apartment Focus camera mode.
 */
USTRUCT(BlueprintType)
struct FApartmentData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Apartment")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Apartment")
	EApartmentStatus Status = EApartmentStatus::Available;

	/** Floor area in square metres. */
	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Apartment")
	float Area = 0.f;

	/** World-space camera location used when focusing this apartment. */
	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Apartment")
	FVector FocusLocation = FVector::ZeroVector;

	/** World-space camera rotation used when focusing this apartment. */
	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Apartment")
	FRotator FocusRotation = FRotator::ZeroRotator;

	FORCEINLINE bool IsSold() const { return Status == EApartmentStatus::Sold; }
};

/**
 * One floor containing apartments. Mirrors an object in the top-level "floors" array.
 * FocusLocation drives the Floor Level camera mode.
 */
USTRUCT(BlueprintType)
struct FFloorData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Floor")
	int32 FloorNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Floor")
	FString DisplayName;

	/** World-space camera location used when focusing this floor. */
	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Floor")
	FVector FocusLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Floor")
	TArray<FApartmentData> Apartments;
};

/** Root configuration object: the whole building. */
USTRUCT(BlueprintType)
struct FBuildingConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Building")
	FString BuildingName;

	UPROPERTY(BlueprintReadOnly, Category = "Configurator|Building")
	TArray<FFloorData> Floors;

	FORCEINLINE bool IsValid() const { return Floors.Num() > 0; }
};
