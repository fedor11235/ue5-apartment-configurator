#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ConfiguratorTypes.h"
#include "FloorPanelWidget.generated.h"

class UPanelWidget;
class UCheckBox;
class UFloorButtonWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloorSelected, int32, FloorNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHideSoldChanged, bool, bHideSold);

/**
 * Left-hand panel that generates one button per floor from the loaded config and hosts
 * the "Hide sold" filter checkbox. Pure UI: it raises OnFloorSelected / OnHideSoldChanged
 * and lets the player controller act on them.
 */
UCLASS(Abstract)
class APARTMENTCONFIGURATOR_API UFloorPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Configurator|UI")
	FOnFloorSelected OnFloorSelected;

	UPROPERTY(BlueprintAssignable, Category = "Configurator|UI")
	FOnHideSoldChanged OnHideSoldChanged;

	/** Clear and rebuild the floor buttons from the config. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|UI")
	void BuildFromConfig(const FBuildingConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "Configurator|UI")
	bool IsHideSoldChecked() const;

protected:
	virtual void NativeConstruct() override;

	/** Container that receives the generated floor buttons (e.g. a VerticalBox). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> FloorButtonContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> HideSoldCheckBox;

	/** Widget class used for each generated floor button (set in the Blueprint). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurator|UI")
	TSubclassOf<UFloorButtonWidget> FloorButtonClass;

private:
	UFUNCTION()
	void HandleFloorButtonClicked(int32 FloorNumber);

	UFUNCTION()
	void HandleHideSoldChanged(bool bIsChecked);
};
