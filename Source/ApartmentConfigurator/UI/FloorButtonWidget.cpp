#include "FloorButtonWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UFloorButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (FloorButton && !FloorButton->OnClicked.IsAlreadyBound(this, &UFloorButtonWidget::HandleClicked))
	{
		FloorButton->OnClicked.AddDynamic(this, &UFloorButtonWidget::HandleClicked);
	}
}

void UFloorButtonWidget::SetFloor(int32 InFloorNumber, const FText& InLabel)
{
	FloorNumber = InFloorNumber;
	if (FloorLabel)
	{
		FloorLabel->SetText(InLabel);
	}
}

void UFloorButtonWidget::HandleClicked()
{
	OnFloorButtonClicked.Broadcast(FloorNumber);
}
