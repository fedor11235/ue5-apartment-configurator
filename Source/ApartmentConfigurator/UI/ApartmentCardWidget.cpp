#include "ApartmentCardWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#define LOCTEXT_NAMESPACE "ApartmentConfigurator"

TSharedRef<SWidget> UApartmentCardWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CardBorder"));
		Root->SetBrushColor(FLinearColor(0.03f, 0.05f, 0.08f, 0.9f));
		Root->SetPadding(FMargin(16.f));

		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CardColumn"));
		Root->AddChild(Column);

		IdText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("IdText"));
		Column->AddChildToVerticalBox(IdText);

		AreaText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AreaText"));
		if (UVerticalBoxSlot* AreaSlot = Column->AddChildToVerticalBox(AreaText))
		{
			AreaSlot->SetPadding(FMargin(0.f, 4.f));
		}

		StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
		Column->AddChildToVerticalBox(StatusText);

		// Action row: Book + Close.
		UHorizontalBox* Actions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Actions"));
		if (UVerticalBoxSlot* ActionsSlot = Column->AddChildToVerticalBox(Actions))
		{
			ActionsSlot->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));
		}

		BookButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BookButton"));
		BookLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BookLabel"));
		BookLabel->SetText(LOCTEXT("Book", "Book"));
		BookButton->AddChild(BookLabel);
		if (UHorizontalBoxSlot* BookSlot = Actions->AddChildToHorizontalBox(BookButton))
		{
			BookSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
		}

		CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
		UTextBlock* CloseLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CloseLabel"));
		CloseLabel->SetText(LOCTEXT("Close", "Close"));
		CloseButton->AddChild(CloseLabel);
		Actions->AddChildToHorizontalBox(CloseButton);

		WidgetTree->RootWidget = Root;
	}

	return Super::RebuildWidget();
}

void UApartmentCardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BookButton && !BookButton->OnClicked.IsAlreadyBound(this, &UApartmentCardWidget::HandleBookClicked))
	{
		BookButton->OnClicked.AddDynamic(this, &UApartmentCardWidget::HandleBookClicked);
	}
	if (CloseButton && !CloseButton->OnClicked.IsAlreadyBound(this, &UApartmentCardWidget::HandleCloseClicked))
	{
		CloseButton->OnClicked.AddDynamic(this, &UApartmentCardWidget::HandleCloseClicked);
	}

	Hide();
}

void UApartmentCardWidget::ShowApartment(const FApartmentData& InData)
{
	CurrentData = InData;

	if (IdText)
	{
		IdText->SetText(FText::FromString(InData.Id));
	}
	if (AreaText)
	{
		AreaText->SetText(FText::Format(
			LOCTEXT("AreaFmt", "{0} m²"), FText::AsNumber(InData.Area)));
	}
	if (StatusText)
	{
		StatusText->SetText(InData.IsSold()
			? LOCTEXT("StatusSold", "Sold")
			: LOCTEXT("StatusAvailable", "Available"));
	}

	// A sold unit cannot be booked.
	if (BookButton)
	{
		BookButton->SetIsEnabled(!InData.IsSold());
	}

	SetVisibility(ESlateVisibility::Visible);
}

void UApartmentCardWidget::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UApartmentCardWidget::HandleBookClicked()
{
	OnBookClicked.Broadcast(CurrentData.Id);
}

void UApartmentCardWidget::HandleCloseClicked()
{
	Hide();
	OnCardClosed.Broadcast();
}

#undef LOCTEXT_NAMESPACE
