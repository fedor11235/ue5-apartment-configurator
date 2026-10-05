#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConfiguratorHUDWidget.generated.h"

class UFloorPanelWidget;
class UApartmentCardWidget;
class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackRequested);

/**
 * Root screen widget. Aggregates the floor panel, the apartment card and the global
 * "Back" button, and exposes the sub-widgets so the player controller can wire signals.
 * The whole hierarchy is built in code (RebuildWidget); no Blueprint design asset required.
 */
UCLASS()
class APARTMENTCONFIGURATOR_API UConfiguratorHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Configurator|UI")
	FOnBackRequested OnBackRequested;

	UFloorPanelWidget* GetFloorPanel() const { return FloorPanel; }
	UApartmentCardWidget* GetApartmentCard() const { return ApartmentCard; }

	/** Enable/disable the Back button (e.g. disabled at the root Genplan view). */
	UFUNCTION(BlueprintCallable, Category = "Configurator|UI")
	void SetBackEnabled(bool bEnabled);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UPROPERTY()
	TObjectPtr<UFloorPanelWidget> FloorPanel;

	UPROPERTY()
	TObjectPtr<UApartmentCardWidget> ApartmentCard;

	UPROPERTY()
	TObjectPtr<UButton> BackButton;

private:
	UFUNCTION()
	void HandleBackClicked();
};
