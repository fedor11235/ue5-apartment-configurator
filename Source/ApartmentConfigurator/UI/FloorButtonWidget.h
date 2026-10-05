#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloorButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloorButtonClicked, int32, FloorNumber);

/**
 * A single auto-generated floor button. One instance is spawned per floor by
 * UFloorPanelWidget. The Button/Label are bound from the Blueprint-designed widget.
 */
UCLASS(Abstract)
class APARTMENTCONFIGURATOR_API UFloorButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnFloorButtonClicked OnFloorButtonClicked;

	UFUNCTION(BlueprintCallable, Category = "Configurator|UI")
	void SetFloor(int32 InFloorNumber, const FText& InLabel);

	int32 GetFloorNumber() const { return FloorNumber; }

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FloorButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FloorLabel;

private:
	UFUNCTION()
	void HandleClicked();

	int32 FloorNumber = 0;
};
