#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataAsset.generated.h"

class UTexture2D;
class UStaticMesh;

/**
 * Data definition for a single item (chair, medicine, key, ...).
 * Created via Content Browser → Miscellaneous → Data Asset → ItemDataAsset.
 * The PrimaryAssetId is built from ItemId so the AssetManager can index them.
 */
UCLASS(BlueprintType)
class PRINCE1_API UItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Stable unique ID. Use lowercase_snake (e.g. "chair", "medicine_king"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable, Category = "Item")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (MultiLine = true))
	FText Description;

	/** Icon shown in inventory / prompt UI. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> Icon;

	/** Mesh used for the world drop (APickupActor visual) and for slot fills. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMesh> WorldMesh;

	/** Marks medicine items so the hub progression system can react to them. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bIsMedicine = false;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("Item"), ItemId);
	}
};
