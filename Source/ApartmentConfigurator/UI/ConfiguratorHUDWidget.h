#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConfiguratorHUDWidget.generated.h"

class UFloorPanelWidget;
class UApartmentCardWidget;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackRequested);

/**
 * Root screen widget. Aggregates the floor panel, the apartment card and the global
 * "Back" button, and exposes the sub-widgets so the player controller can wire signals.
 */
UCLASS(Abstract)
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
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFloorPanelWidget> FloorPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UApartmentCardWidget> ApartmentCard;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

private:
	UFUNCTION()
	void HandleBackClicked();
};
