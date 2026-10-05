#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/ConfiguratorTypes.h"
#include "ApartmentActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class AApartmentActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnApartmentClicked, AApartmentActor*, Apartment);

/**
 * One selectable apartment volume in the 3D scene. The mesh is assigned in a Blueprint
 * subclass; this class owns the data binding, click forwarding and visual state
 * (highlight on hover/focus, dimmed + non-interactive when sold or hidden by the filter).
 *
 * Click detection relies on APlayerController::bEnableClickEvents, enabled by
 * AConfiguratorPlayerController.
 */
UCLASS(Blueprintable)
class APARTMENTCONFIGURATOR_API AApartmentActor : public AActor
{
	GENERATED_BODY()

public:
	AApartmentActor();

	/** Broadcast when this apartment is clicked and interaction is allowed. */
	UPROPERTY(BlueprintAssignable, Category = "Configurator|Interaction")
	FOnApartmentClicked OnApartmentClicked;

	/**
	 * Id of the config entry this placed actor represents (set per-instance in the level).
	 * The player controller uses it to look up the matching apartment in the loaded JSON.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator|Interaction")
	FString ApartmentId;

	/** Bind the actor to its data. Applies the correct initial visual state. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Interaction")
	void InitializeFromData(const FApartmentData& InData);

	UFUNCTION(BlueprintCallable, Category = "Configurator|Interaction")
	const FApartmentData& GetData() const { return Data; }

	UFUNCTION(BlueprintCallable, Category = "Configurator|Interaction")
	const FString& GetApartmentId() const { return ApartmentId; }

	UFUNCTION(BlueprintCallable, Category = "Configurator|Interaction")
	bool IsInteractable() const { return bInteractable; }

	/** Hidden-by-filter apartments stay visible but become dimmed and non-clickable. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Interaction")
	void SetFilteredOut(bool bFilteredOut);

	UFUNCTION(BlueprintCallable, Category = "Configurator|Interaction")
	void SetHighlighted(bool bHighlighted);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Configurator")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Base tint by state, pushed to the dynamic material's "BaseColor" parameter. */
	UPROPERTY(EditAnywhere, Category = "Configurator|Appearance")
	FLinearColor AvailableColor = FLinearColor(0.15f, 0.55f, 0.85f);

	UPROPERTY(EditAnywhere, Category = "Configurator|Appearance")
	FLinearColor SoldColor = FLinearColor(0.35f, 0.35f, 0.35f);

	UPROPERTY(EditAnywhere, Category = "Configurator|Appearance")
	FLinearColor HighlightColor = FLinearColor(1.0f, 0.75f, 0.2f);

	/** Opacity param ("Opacity") applied when the apartment is filtered out. */
	UPROPERTY(EditAnywhere, Category = "Configurator|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FilteredOpacity = 0.25f;

private:
	UFUNCTION()
	void HandleMeshClicked(UPrimitiveComponent* ClickedComp, FKey ButtonPressed);

	UFUNCTION()
	void HandleBeginCursorOver(UPrimitiveComponent* Comp);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* Comp);

	/** Recompute and push colour/opacity to the dynamic material instance. */
	void RefreshAppearance();

	UPROPERTY()
	FApartmentData Data;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynMaterial;

	bool bInteractable = true;
	bool bFiltered = false;
	bool bHovered = false;
	bool bHighlighted = false;
};
