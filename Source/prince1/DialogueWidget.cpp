#include "DialogueWidget.h"

#include "DialogueDataAsset.h"

void UDialogueWidget::StartDialogue(const FText& NPCName, UDialogueDataAsset* InDialogue)
{
	if (!InDialogue || InDialogue->Lines.Num() == 0)
	{
		// Nothing to play — degrade to a clean no-op rather than half-opening UI.
		return;
	}

	Dialogue         = InDialogue;
	CachedNPCName    = NPCName;
	CurrentLineIndex = 0;
	bActive          = true;

	ShowLine(0);
}

void UDialogueWidget::Advance()
{
	if (!bActive)
	{
		return;
	}

	++CurrentLineIndex;

	const int32 LineCount = Dialogue ? Dialogue->Lines.Num() : 0;
	if (CurrentLineIndex >= LineCount)
	{
		EndDialogue();
		return;
	}

	ShowLine(CurrentLineIndex);
}

void UDialogueWidget::EndDialogue()
{
	if (!bActive)
	{
		return;
	}

	bActive          = false;
	Dialogue         = nullptr;
	CurrentLineIndex = 0;

	// Notify listeners (PrinceCharacter) BEFORE the WBP hook so any C++-side
	// state (bIsInDialogue) clears in time for the WBP to safely rebuild UI.
	OnDialogueClosed.Broadcast();
	OnDialogueEnded();
}

void UDialogueWidget::NativeDestruct()
{
	OnDialogueClosed.Clear();

	Super::NativeDestruct();
}

void UDialogueWidget::ShowLine(int32 Index)
{
	if (!Dialogue || !Dialogue->Lines.IsValidIndex(Index))
	{
		return;
	}

	const FDialogueLine& Line = Dialogue->Lines[Index];

	// Empty per-line speaker → fall back to the NPC's display name.
	const FText Speaker = Line.Speaker.IsEmpty() ? CachedNPCName : Line.Speaker;
	const bool  bLast   = (Index == Dialogue->Lines.Num() - 1);

	OnLineDisplayed(Speaker, Line.Text, bLast);
}
