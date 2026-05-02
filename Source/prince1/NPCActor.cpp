#include "NPCActor.h"

#include "DialogueDataAsset.h"
#include "PrinceCharacter.h"

#include "Components/StaticMeshComponent.h"

#define LOCTEXT_NAMESPACE "NPC"

ANPCActor::ANPCActor()
{
	// NPCs are solid props — block the capsule so the player can't walk through.
	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	}
}

bool ANPCActor::CanInteract_Implementation(const AActor* Interactor) const
{
	if (!Interactor || !InitialDialogue)
	{
		return false;
	}

	// Suppress the prompt while the player is already in a dialogue (with this
	// NPC or any other) so [E] presses route to Advance(), not a re-trigger.
	if (const APrinceCharacter* PrinceChar = Cast<APrinceCharacter>(Interactor))
	{
		if (PrinceChar->IsInDialogue())
		{
			return false;
		}
	}

	return true;
}

FText ANPCActor::GetInteractPrompt_Implementation() const
{
	if (!TalkPrompt.IsEmpty())
	{
		return TalkPrompt;
	}

	if (!NPCName.IsEmpty())
	{
		return FText::Format(LOCTEXT("TalkToFmt", "Talk to {0}"), NPCName);
	}

	return LOCTEXT("TalkPrompt", "Talk");
}

bool ANPCActor::OnInteract_Implementation(AActor* Interactor)
{
	APrinceCharacter* PrinceChar = Cast<APrinceCharacter>(Interactor);
	if (!PrinceChar || PrinceChar->IsInDialogue())
	{
		return false;
	}

	// On the second-and-later visits, prefer RepeatDialogue if the designer
	// supplied one. Otherwise fall back to the same intro.
	UDialogueDataAsset* ToPlay = (bHasTalked && RepeatDialogue) ? RepeatDialogue : InitialDialogue;
	if (!ToPlay || ToPlay->Lines.Num() == 0)
	{
		return false;
	}

	PrinceChar->BeginDialogue(NPCName, ToPlay);
	bHasTalked = true;
	return true;
}

#undef LOCTEXT_NAMESPACE
