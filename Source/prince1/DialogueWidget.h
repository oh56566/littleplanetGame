#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidget.generated.h"

class UDialogueDataAsset;

// Non-dynamic multicast: doesn't go through the UFunction reflection layer,
// which sidesteps stale-binding crashes after Live Coding patches and avoids
// the ProcessEvent dispatch overhead. C++ listeners only — BP can still drive
// playback via the UFUNCTION StartDialogue/Advance/EndDialogue methods, and
// react via OnDialogueEnded (BlueprintImplementableEvent).
DECLARE_MULTICAST_DELEGATE(FOnDialogueClosed);

/**
 * Linear dialogue presenter. The C++ base owns sequencing; the WBP subclass
 * owns visuals via the BlueprintImplementableEvents below. Recommended layout:
 * a centred / bottom panel with a small "Speaker" label and a large "Body"
 * text block, plus a "▶ E" hint glyph.
 *
 * Lifecycle:
 *   StartDialogue → ShowLine(0) → (Advance × N) → EndDialogue → broadcast OnDialogueClosed
 */
UCLASS()
class PRINCE1_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Begins playback. Resets to line 0, sets active, fires OnLineDisplayed for the first line. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(const FText& NPCName, UDialogueDataAsset* InDialogue);

	/** Advances to the next line. End if past the last; safe to call when inactive (no-op). */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void Advance();

	/** Force-closes the dialogue. WBP can bind this to ESC if desired. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsDialogueActive() const { return bActive; }

	/**
	 * WBP hook — called whenever a new line should be visible.
	 * The WBP should set its SpeakerText/BodyText UMG widgets and ensure the
	 * panel is shown. bIsLastLine is a hint (e.g. swap "▶ E" for "✕ E").
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnLineDisplayed(const FText& Speaker, const FText& Text, bool bIsLastLine);

	/** WBP hook — called once after EndDialogue. Use to hide the panel. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueEnded();

	/** Listeners (PrinceCharacter) get notified when the dialogue closes. */
	FOnDialogueClosed OnDialogueClosed;

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<UDialogueDataAsset> Dialogue;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Dialogue")
	FText CachedNPCName;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Dialogue")
	int32 CurrentLineIndex = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Dialogue")
	bool bActive = false;

private:
	/** Resolves the speaker (NPC fallback) and dispatches OnLineDisplayed. */
	void ShowLine(int32 Index);
};
