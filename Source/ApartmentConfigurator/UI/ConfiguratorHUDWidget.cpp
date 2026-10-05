#include "ConfiguratorHUDWidget.h"
#include "FloorPanelWidget.h"
#include "ApartmentCardWidget.h"

#include "Components/Button.h"

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
