#include "ConfiguratorHUDWidget.h"
#include "FloorPanelWidget.h"
#include "ApartmentCardWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "ApartmentConfigurator"

TSharedRef<SWidget> UConfiguratorHUDWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = Canvas;

		// Back button — top-left.
		BackButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BackButton"));
		UTextBlock* BackLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BackLabel"));
		BackLabel->SetText(LOCTEXT("Back", "< Back"));
		BackButton->AddChild(BackLabel);
		if (UCanvasPanelSlot* BackSlot = Canvas->AddChildToCanvas(BackButton))
		{
			BackSlot->SetAnchors(FAnchors(0.f, 0.f));
			BackSlot->SetPosition(FVector2D(24.f, 24.f));
			BackSlot->SetSize(FVector2D(120.f, 40.f));
		}

		// Floor panel — left edge, below the Back button.
		FloorPanel = WidgetTree->ConstructWidget<UFloorPanelWidget>(UFloorPanelWidget::StaticClass(), TEXT("FloorPanel"));
		if (UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(FloorPanel))
		{
			PanelSlot->SetAnchors(FAnchors(0.f, 0.f));
			PanelSlot->SetPosition(FVector2D(24.f, 80.f));
			PanelSlot->SetSize(FVector2D(240.f, 440.f));
		}

		// Apartment card — right edge.
		ApartmentCard = WidgetTree->ConstructWidget<UApartmentCardWidget>(UApartmentCardWidget::StaticClass(), TEXT("ApartmentCard"));
		if (UCanvasPanelSlot* CardSlot = Canvas->AddChildToCanvas(ApartmentCard))
		{
			CardSlot->SetAnchors(FAnchors(1.f, 0.f));
			CardSlot->SetAlignment(FVector2D(1.f, 0.f));
			CardSlot->SetPosition(FVector2D(-24.f, 80.f));
			CardSlot->SetSize(FVector2D(300.f, 220.f));
		}
	}

	return Super::RebuildWidget();
}

void UConfiguratorHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton && !BackButton->OnClicked.IsAlreadyBound(this, &UConfiguratorHUDWidget::HandleBackClicked))
	{
		BackButton->OnClicked.AddDynamic(this, &UConfiguratorHUDWidget::HandleBackClicked);
	}
	SetBackEnabled(false);
}

void UConfiguratorHUDWidget::SetBackEnabled(bool bEnabled)
{
	if (BackButton)
	{
		BackButton->SetIsEnabled(bEnabled);
	}
}

void UConfiguratorHUDWidget::HandleBackClicked()
{
	OnBackRequested.Broadcast();
}

#undef LOCTEXT_NAMESPACE
