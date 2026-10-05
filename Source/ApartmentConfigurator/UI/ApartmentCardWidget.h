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
 * Layout is constructed in code (RebuildWidget); no Blueprint design asset is required.
 */
UCLASS()
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
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UPROPERTY()
	TObjectPtr<UTextBlock> IdText;

	UPROPERTY()
	TObjectPtr<UTextBlock> AreaText;

	UPROPERTY()
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY()
	TObjectPtr<UButton> BookButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> BookLabel;

	UPROPERTY()
	TObjectPtr<UButton> CloseButton;

private:
	UFUNCTION()
	void HandleBookClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UPROPERTY()
	FApartmentData CurrentData;
};
