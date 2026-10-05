#include "ApartmentActor.h"
#include "ApartmentConfigurator.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AApartmentActor::AApartmentActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// Required for OnClicked / cursor-over events to fire on this primitive.
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->bVisibleInReflectionCaptures = true;
}

void AApartmentActor::BeginPlay()
{
	Super::BeginPlay();

	Mesh->OnClicked.AddDynamic(this, &AApartmentActor::HandleMeshClicked);
	Mesh->OnBeginCursorOver.AddDynamic(this, &AApartmentActor::HandleBeginCursorOver);
	Mesh->OnEndCursorOver.AddDynamic(this, &AApartmentActor::HandleEndCursorOver);

	// Create a dynamic material instance from slot 0 so we can recolour per state at runtime.
	if (Mesh->GetMaterial(0))
	{
		DynMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	else
	{
		UE_LOG(LogConfigurator, Warning,
			TEXT("ApartmentActor '%s' has no material on slot 0 — tinting will be skipped."), *GetName());
	}

	RefreshAppearance();
}

void AApartmentActor::InitializeFromData(const FApartmentData& InData)
{
	Data = InData;
	// Sold apartments are never interactable, regardless of the filter.
	bInteractable = !Data.IsSold();
	RefreshAppearance();
}

void AApartmentActor::SetFilteredOut(bool bFilteredOut)
{
	bFiltered = bFilteredOut;
	// Filtered or sold → not clickable.
	bInteractable = !bFiltered && !Data.IsSold();
	RefreshAppearance();
}

void AApartmentActor::SetHighlighted(bool bInHighlighted)
{
	bHighlighted = bInHighlighted;
	RefreshAppearance();
}

void AApartmentActor::HandleMeshClicked(UPrimitiveComponent* /*ClickedComp*/, FKey ButtonPressed)
{
	if (ButtonPressed != EKeys::LeftMouseButton)
	{
		return;
	}
	if (!bInteractable)
	{
		UE_LOG(LogConfigurator, Verbose, TEXT("Click ignored on non-interactable apartment '%s'."), *Data.Id);
		return;
	}
	OnApartmentClicked.Broadcast(this);
}

void AApartmentActor::HandleBeginCursorOver(UPrimitiveComponent* /*Comp*/)
{
	bHovered = true;
	RefreshAppearance();
}

void AApartmentActor::HandleEndCursorOver(UPrimitiveComponent* /*Comp*/)
{
	bHovered = false;
	RefreshAppearance();
}

void AApartmentActor::RefreshAppearance()
{
	if (!DynMaterial)
	{
		return;
	}

	FLinearColor Color = Data.IsSold() ? SoldColor : AvailableColor;
	if (bInteractable && (bHighlighted || bHovered))
	{
		Color = HighlightColor;
	}

	DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
	DynMaterial->SetScalarParameterValue(TEXT("Opacity"), bFiltered ? FilteredOpacity : 1.0f);
}
