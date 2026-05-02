#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UInventorySlotWidget;
class UItemDataAsset;
class UPanelWidget;

/**
 * Always-visible HUD widget that mirrors the contents of a UInventoryComponent.
 * Subscribes to OnInventoryChanged and rebuilds the slot list each time an item
 * is added, removed, or consumed (e.g. placed into a slot).
 *
 * The WBP must contain a UPanelWidget named exactly "SlotsContainer"
 * (VerticalBox / HorizontalBox / WrapBox / GridPanel — anything that accepts
 * children). The bound inventory's entries are rendered as instances of
 * SlotWidgetClass added to that container.
 */
UCLASS()
class PRINCE1_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Bind to an inventory component. Subscribes to its change delegate and rebuilds slots immediately. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void BindToInventory(UInventoryComponent* InInventory);

	/** WBP class spawned for each inventory entry. Set in WBP defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

protected:
	virtual void NativeDestruct() override;

	/** Container the slots are added to. Bound from the WBP by name. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> SlotsContainer;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	TArray<TObjectPtr<UInventorySlotWidget>> SpawnedSlots;

	/** Override in WBP if you want to react to a full rebuild (e.g. play a flash). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnInventoryRebuilt(int32 SlotCount);

private:
	UFUNCTION()
	void HandleInventoryChanged(UItemDataAsset* Item, int32 NewCount);

	void RebuildSlots();
	void ClearSlots();
};
