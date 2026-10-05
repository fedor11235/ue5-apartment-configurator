#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ConfiguratorTypes.h"
#include "ApartmentCardWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBookClicked, const FString&, ApartmentId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCardClosed);

/**
 * Detail card for a single apartment: shows id, area and status, exposes a "Book" button
 * (disabled when the unit is sold) and a close button. Shown when an apartment is focused.
 */
UCLASS(Abstract)
class APARTMENTCONFIGURATOR_API UApartmentCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Configurator|UI")
	FOnBookClicked OnBookClicked;

	UPROPERTY(BlueprintAssignable, Category = "Configurator|UI")
	FOnCardClosed OnCardClosed;

	/** Populate the card from data and make it visible. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|UI")
	void ShowApartment(const FApartmentData& InData);

	UFUNCTION(BlueprintCallable, Category = "Configurator|UI")
	void Hide();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> IdText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AreaText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BookButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

private:
	UFUNCTION()
	void HandleBookClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UPROPERTY()
	FApartmentData CurrentData;
};
