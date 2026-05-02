#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implement on actors the player can interact with via the prompt + E key.
 * All methods are BlueprintNativeEvent so subclasses can override in C++ or BP.
 */
class PRINCE1_API IInteractable
{
	GENERATED_BODY()

public:
	/**
	 * Called when the player presses Interact while this is the focused target.
	 * Return true if the interaction was handled (consumes the input).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool OnInteract(AActor* Interactor);
	virtual bool OnInteract_Implementation(AActor* Interactor) { return false; }

	/** Localised verb shown in the prompt UI ("Pick up", "Talk", "Place"). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractPrompt() const;
	virtual FText GetInteractPrompt_Implementation() const
	{
		return NSLOCTEXT("Interaction", "DefaultPrompt", "Interact");
	}

	/** Whether this object currently accepts interaction (e.g. already used / locked). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(const AActor* Interactor) const;
	virtual bool CanInteract_Implementation(const AActor* Interactor) const { return true; }
};
