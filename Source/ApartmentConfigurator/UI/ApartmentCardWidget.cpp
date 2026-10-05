#include "ApartmentCardWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "ApartmentConfigurator"

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
