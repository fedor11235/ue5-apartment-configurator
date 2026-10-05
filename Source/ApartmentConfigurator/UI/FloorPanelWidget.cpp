#include "FloorPanelWidget.h"
#include "FloorButtonWidget.h"
#include "ApartmentConfigurator.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#define LOCTEXT_NAMESPACE "ApartmentConfigurator"

TSharedRef<SWidget> UFloorPanelWidget::RebuildWidget()
{
	if (!FloorButtonClass)
	{
		FloorButtonClass = UFloorButtonWidget::StaticClass();
	}

	if (!WidgetTree->RootWidget)
	{
		UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PanelBorder"));
		Root->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.04f, 0.75f));
		Root->SetPadding(FMargin(12.f));

		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
		Root->AddChild(Column);

		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
		Title->SetText(LOCTEXT("FloorsTitle", "FLOORS"));
		Column->AddChildToVerticalBox(Title);

		UVerticalBox* ButtonBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("FloorButtonContainer"));
		FloorButtonContainer = ButtonBox;
		if (UVerticalBoxSlot* BoxSlot = Column->AddChildToVerticalBox(ButtonBox))
		{
			BoxSlot->SetPadding(FMargin(0.f, 8.f));
		}

		// "Hide sold" filter row: checkbox + label.
		UHorizontalBox* FilterRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("FilterRow"));
		Column->AddChildToVerticalBox(FilterRow);

		HideSoldCheckBox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), TEXT("HideSoldCheckBox"));
		FilterRow->AddChildToHorizontalBox(HideSoldCheckBox);

		UTextBlock* FilterLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FilterLabel"));
		FilterLabel->SetText(LOCTEXT("HideSold", "Hide sold"));
		if (UHorizontalBoxSlot* LabelSlot = FilterRow->AddChildToHorizontalBox(FilterLabel))
		{
			LabelSlot->SetPadding(FMargin(6.f, 0.f, 0.f, 0.f));
			LabelSlot->SetVerticalAlignment(VAlign_Center);
		}

		WidgetTree->RootWidget = Root;
	}

	return Super::RebuildWidget();
}

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
		UE_LOG(LogConfigurator, Error, TEXT("FloorPanelWidget has no FloorButtonContainer."));
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

#undef LOCTEXT_NAMESPACE
