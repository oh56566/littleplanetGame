#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

class UItemDataAsset;

/**
 * Single item slot. The C++ base just stores the (Item, Count) pair and
 * notifies the WBP subclass via OnEntryUpdated; the WBP designs the visual
 * (icon, count label, name, hover effects, ...).
 */
UCLASS()
class PRINCE1_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called by UInventoryWidget after spawning / when the count changes. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetEntry(UItemDataAsset* InItem, int32 InCount);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItemDataAsset* GetItem() const { return Item; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetCount() const { return Count; }

	/**
	 * Override in WBP. Receives the new (Item, Count) pair so the designer can
	 * update the icon, name, count text, etc. Item == null means "clear / hide".
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnEntryUpdated(UItemDataAsset* NewItem, int32 NewCount);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UItemDataAsset> Item;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Count = 0;
};
