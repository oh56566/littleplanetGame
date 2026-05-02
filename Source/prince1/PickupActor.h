#pragma once

#include "CoreMinimal.h"
#include "PlanetSurfaceActor.h"
#include "Interactable.h"
#include "PickupActor.generated.h"

class UItemDataAsset;

/**
 * A world drop. Inherits surface-alignment from APlanetSurfaceActor and
 * implements IInteractable to be picked up by a UInventoryComponent owner.
 *
 * Set Item in the Details panel — the visual mesh auto-syncs from
 * Item->WorldMesh on construction.
 */
UCLASS()
class PRINCE1_API APickupActor : public APlanetSurfaceActor, public IInteractable
{
	GENERATED_BODY()

public:
	APickupActor();

	// IInteractable
	virtual bool  OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation() const override;
	virtual bool  CanInteract_Implementation(const AActor* Interactor) const override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UItemDataAsset> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "1"))
	int32 Amount = 1;

	/** If true, destroy the actor after a successful pickup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bDestroyOnPickup = true;
};
