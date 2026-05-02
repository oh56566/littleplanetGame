#pragma once

#include "CoreMinimal.h"
#include "PlanetSurfaceActor.h"
#include "Interactable.h"
#include "PlacementSlotActor.generated.h"

class UItemDataAsset;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotFilled, AActor*, Filler);

/**
 * A planet-surface slot that accepts a specific UItemDataAsset.
 * On a successful interaction it consumes the item from the interactor's
 * inventory, swaps the placed mesh in, and broadcasts OnSlotFilled — which
 * the planet's clear-condition logic listens for.
 */
UCLASS()
class PRINCE1_API APlacementSlotActor : public APlanetSurfaceActor, public IInteractable
{
	GENERATED_BODY()

public:
	APlacementSlotActor();

	// IInteractable
	virtual bool  OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation() const override;
	virtual bool  CanInteract_Implementation(const AActor* Interactor) const override;

	UFUNCTION(BlueprintPure, Category = "Slot")
	bool IsFilled() const { return bFilled; }

	UPROPERTY(BlueprintAssignable, Category = "Slot")
	FOnSlotFilled OnSlotFilled;

protected:
	virtual void BeginPlay() override;

	/** Item that the slot accepts. Required. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slot")
	TObjectPtr<UItemDataAsset> RequiredItem;

	/** Optional override mesh shown after fill. Falls back to RequiredItem->WorldMesh. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slot")
	TObjectPtr<UStaticMesh> FilledMeshOverride;

	/** Visual shown when filled. Hidden until then. Spawned in constructor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Slot")
	TObjectPtr<UStaticMeshComponent> PlacedMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Slot")
	bool bFilled = false;

private:
	void ApplyFilledVisual(bool bShow);
};
