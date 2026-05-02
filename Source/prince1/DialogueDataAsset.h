#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueDataAsset.generated.h"

/**
 * One line spoken in a dialogue. Played sequentially in v1; NodeId is reserved
 * for v2 branching (the player picks a choice that targets a NodeId).
 */
USTRUCT(BlueprintType)
struct FDialogueLine
{
	GENERATED_BODY()

	/** Speaker label. Leave empty to inherit the owning NPC's name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Speaker;

	/** Body text. Multi-line allowed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (MultiLine = true))
	FText Text;

	/** Optional ID for v2 branching targets. v1 ignores this. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName NodeId;
};

/**
 * v2 placeholder — choice button that jumps to a target NodeId. Defined now so
 * the data asset structure is forward-compatible without re-saving content.
 */
USTRUCT(BlueprintType)
struct FDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText ChoiceText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName GotoNodeId;
};

/**
 * A single dialogue tree for an NPC interaction. v1 plays Lines linearly.
 *
 * Created via Content Browser → Miscellaneous → Data Asset → DialogueDataAsset.
 * The PrimaryAssetId is built from DialogueId so the AssetManager can index them.
 */
UCLASS(BlueprintType)
class PRINCE1_API UDialogueDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Stable unique ID. Use lowercase_snake (e.g. "dialogue_king_intro"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AssetRegistrySearchable, Category = "Dialogue")
	FName DialogueId;

	/** Ordered list of lines. v1 plays them top-to-bottom. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueLine> Lines;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("Dialogue"), DialogueId);
	}
};
