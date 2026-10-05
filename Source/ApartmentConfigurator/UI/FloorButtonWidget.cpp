#include "FloorButtonWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> UFloorButtonWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		FloorButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("FloorButton"));
		FloorLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FloorLabel"));
		FloorButton->AddChild(FloorLabel);
		WidgetTree->RootWidget = FloorButton;
	}

	return Super::RebuildWidget();
}

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
