#include "FloorPanelWidget.h"
#include "FloorButtonWidget.h"
#include "ApartmentConfigurator.h"

#include "Components/CheckBox.h"
#include "Components/PanelWidget.h"

void UFloorPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HideSoldCheckBox && !HideSoldCheckBox->OnCheckStateChanged.IsAlreadyBound(this, &UFloorPanelWidget::HandleHideSoldChanged))
	{
		HideSoldCheckBox->OnCheckStateChanged.AddDynamic(this, &UFloorPanelWidget::HandleHideSoldChanged);
	}
}

void UFloorPanelWidget::BuildFromConfig(const FBuildingConfig& Config)
{
	if (!FloorButtonContainer)
	{
		UE_LOG(LogConfigurator, Error, TEXT("FloorPanelWidget has no bound FloorButtonContainer."));
		return;
	}
	if (!FloorButtonClass)
	{
		UE_LOG(LogConfigurator, Error, TEXT("FloorPanelWidget.FloorButtonClass is not set — cannot generate buttons."));
		return;
	}

	FloorButtonContainer->ClearChildren();

	// Top floor first reads naturally in a vertical list.
	TArray<FFloorData> Sorted = Config.Floors;
	Sorted.Sort([](const FFloorData& A, const FFloorData& B) { return A.FloorNumber > B.FloorNumber; });

	for (const FFloorData& Floor : Sorted)
	{
		UFloorButtonWidget* Button = CreateWidget<UFloorButtonWidget>(this, FloorButtonClass);
		if (!Button)
		{
			continue;
		}
		Button->SetFloor(Floor.FloorNumber, FText::FromString(Floor.DisplayName));
		Button->OnFloorButtonClicked.AddDynamic(this, &UFloorPanelWidget::HandleFloorButtonClicked);
		FloorButtonContainer->AddChild(Button);
	}

	UE_LOG(LogConfigurator, Log, TEXT("FloorPanel built with %d floor button(s)."), Sorted.Num());
}

bool UFloorPanelWidget::IsHideSoldChecked() const
{
	return HideSoldCheckBox ? HideSoldCheckBox->IsChecked() : false;
}

void UFloorPanelWidget::HandleFloorButtonClicked(int32 FloorNumber)
{
	OnFloorSelected.Broadcast(FloorNumber);
}

void UFloorPanelWidget::HandleHideSoldChanged(bool bIsChecked)
{
	OnHideSoldChanged.Broadcast(bIsChecked);
}
