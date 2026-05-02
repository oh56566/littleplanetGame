#include "PickupActor.h"

#include "InventoryComponent.h"
#include "ItemDataAsset.h"

#include "Components/StaticMeshComponent.h"

#define LOCTEXT_NAMESPACE "Pickup"

APickupActor::APickupActor()
{
	// Mesh comes from the inherited APlanetSurfaceActor::Mesh component.
	// Pickups don't block movement — let the player walk over / through them.
	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);
		Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}

void APickupActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Auto-apply the Item's WorldMesh to the visual component so designers
	// only have to pick the item asset.
	if (Mesh && Item && Item->WorldMesh && Mesh->GetStaticMesh() != Item->WorldMesh)
	{
		Mesh->SetStaticMesh(Item->WorldMesh);
	}
}

bool APickupActor::OnInteract_Implementation(AActor* Interactor)
{
	if (!Interactor || !Item)
	{
		return false;
	}

	UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!Inv)
	{
		return false;
	}

	Inv->AddItem(Item, Amount);

	if (bDestroyOnPickup)
	{
		Destroy();
	}
	return true;
}

FText APickupActor::GetInteractPrompt_Implementation() const
{
	if (Item && !Item->DisplayName.IsEmpty())
	{
		return FText::Format(LOCTEXT("PickupPromptFmt", "Pick up {0}"), Item->DisplayName);
	}
	return LOCTEXT("PickupPrompt", "Pick up");
}

bool APickupActor::CanInteract_Implementation(const AActor* Interactor) const
{
	return Item != nullptr && Interactor != nullptr;
}

#undef LOCTEXT_NAMESPACE
