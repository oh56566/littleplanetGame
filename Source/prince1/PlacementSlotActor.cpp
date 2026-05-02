#include "PlacementSlotActor.h"

#include "InventoryComponent.h"
#include "ItemDataAsset.h"

#include "Components/StaticMeshComponent.h"

#define LOCTEXT_NAMESPACE "Slot"

APlacementSlotActor::APlacementSlotActor()
{
	// Empty marker mesh from the inherited APlanetSurfaceActor — designers can
	// assign a small "outline / decal" mesh in the BP to hint where to place.

	PlacedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlacedMesh"));
	PlacedMesh->SetupAttachment(SceneRoot);
	PlacedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlacedMesh->SetVisibility(false);
}

void APlacementSlotActor::BeginPlay()
{
	Super::BeginPlay();
	ApplyFilledVisual(bFilled);
}

bool APlacementSlotActor::OnInteract_Implementation(AActor* Interactor)
{
	if (bFilled || !Interactor || !RequiredItem)
	{
		return false;
	}

	UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!Inv || !Inv->RemoveItem(RequiredItem, 1))
	{
		return false;
	}

	bFilled = true;
	ApplyFilledVisual(true);
	OnSlotFilled.Broadcast(Interactor);
	return true;
}

FText APlacementSlotActor::GetInteractPrompt_Implementation() const
{
	if (bFilled)
	{
		return FText::GetEmpty(); // No prompt when already filled.
	}
	if (RequiredItem && !RequiredItem->DisplayName.IsEmpty())
	{
		return FText::Format(LOCTEXT("PlacePromptFmt", "Place {0}"), RequiredItem->DisplayName);
	}
	return LOCTEXT("PlacePrompt", "Place");
}

bool APlacementSlotActor::CanInteract_Implementation(const AActor* Interactor) const
{
	if (bFilled || !RequiredItem || !Interactor)
	{
		return false;
	}
	const UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();
	return Inv && Inv->HasItem(RequiredItem, 1);
}

void APlacementSlotActor::ApplyFilledVisual(bool bShow)
{
	if (!PlacedMesh)
	{
		return;
	}

	if (bShow)
	{
		UStaticMesh* MeshToShow = FilledMeshOverride ? FilledMeshOverride.Get()
			: (RequiredItem ? RequiredItem->WorldMesh.Get() : nullptr);
		if (MeshToShow && PlacedMesh->GetStaticMesh() != MeshToShow)
		{
			PlacedMesh->SetStaticMesh(MeshToShow);
		}
	}
	PlacedMesh->SetVisibility(bShow);
}

#undef LOCTEXT_NAMESPACE
