#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractPromptWidget.generated.h"

/**
 * On-screen prompt for the focused interactable. The C++ base only stores state
 * and notifies the Blueprint subclass; the WBP designs the actual visuals
 * (recommended layout: a centred horizontal box with a key glyph + label).
 */
UCLASS()
class PRINCE1_API UInteractPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Updates the prompt. Empty Text = hide.
	 * The character's interact trace calls this every frame with the current target.
	 */
	UFUNCTION(BlueprintCallable, Category = "Prompt")
	void SetPrompt(const FText& Text);

	UFUNCTION(BlueprintPure, Category = "Prompt")
	bool IsPromptVisible() const { return bIsVisible; }

	UFUNCTION(BlueprintPure, Category = "Prompt")
	const FText& GetPrompt() const { return CurrentPrompt; }

	/**
	 * Override in WBP to update the visual state. Receives the new text and a
	 * visibility flag. Called only when the visible/text state actually changes.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Prompt")
	void OnPromptUpdated(const FText& NewText, bool bVisible);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Prompt")
	FText CurrentPrompt;

	UPROPERTY(BlueprintReadOnly, Category = "Prompt")
	bool bIsVisible = false;
};
