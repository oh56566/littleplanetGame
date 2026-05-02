#include "InventorySlotWidget.h"

void UInventorySlotWidget::SetEntry(UItemDataAsset* InItem, int32 InCount)
{
	Item  = InItem;
	Count = InCount;
	OnEntryUpdated(Item, Count);
}
