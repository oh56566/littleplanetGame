#pragma once

#include "CoreMinimal.h"
#include "PlanetSurfaceActor.h"
#include "Interactable.h"
#include "NPCActor.generated.h"

class UDialogueDataAsset;

/**
 * Surface-aligned dialogue NPC. Drop on a planet, assign a name and a dialogue
 * data asset; talking to it routes the player into UDialogueWidget playback.
 *
 * Tracks bHasTalked so designers can branch into RepeatDialogue on subsequent
 * conversations (e.g. "오, 또 만났구나!" instead of the intro).
 */
UCLASS()
class PRINCE1_API ANPCActor : public APlanetSurfaceActor, public IInteractable
{
	GENERATED_BODY()

public:
	ANPCActor();

	// IInteractable
	virtual bool  OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation() const override;
	virtual bool  CanInteract_Implementation(const AActor* Interactor) const override;

protected:
	/** Display name shown in dialogue header and the default interact prompt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FText NPCName;

	/** Played the first time the player interacts. Required — null = no interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UDialogueDataAsset> InitialDialogue;

	/** Optional — played on every interaction after the first. null = repeat InitialDialogue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UDialogueDataAsset> RepeatDialogue;

	/**
	 * Custom interact prompt. Leave empty to auto-format
	 * "Talk to {NPCName}" (or just "Talk" if NPCName is empty too).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FText TalkPrompt;

	/** Becomes true after the first successful interaction. Drives RepeatDialogue branch. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "NPC")
	bool bHasTalked = false;
};
