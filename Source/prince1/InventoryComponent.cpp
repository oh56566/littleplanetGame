#include "InventoryComponent.h"

#include "ItemDataAsset.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::AddItem(UItemDataAsset* Item, int32 Count)
{
	if (!Item || Count <= 0)
	{
		return;
	}

	const int32 Index = FindEntryIndex(Item);
	int32 NewCount;
	if (Index == INDEX_NONE)
	{
		FInventoryEntry Entry;
		Entry.Item  = Item;
		Entry.Count = Count;
		Items.Add(Entry);
		NewCount = Count;
	}
	else
	{
		Items[Index].Count += Count;
		NewCount = Items[Index].Count;
	}

	OnInventoryChanged.Broadcast(Item, NewCount);
}

bool UInventoryComponent::RemoveItem(UItemDataAsset* Item, int32 Count)
{
	if (!Item || Count <= 0)
	{
		return false;
	}

	const int32 Index = FindEntryIndex(Item);
	if (Index == INDEX_NONE || Items[Index].Count < Count)
	{
		return false;
	}

	Items[Index].Count -= Count;
	const int32 NewCount = Items[Index].Count;
	if (NewCount <= 0)
	{
		Items.RemoveAt(Index);
	}

	OnInventoryChanged.Broadcast(Item, NewCount);
	return true;
}

int32 UInventoryComponent::GetItemCount(UItemDataAsset* Item) const
{
	const int32 Index = FindEntryIndex(Item);
	return Index == INDEX_NONE ? 0 : Items[Index].Count;
}

bool UInventoryComponent::HasItem(UItemDataAsset* Item, int32 MinCount) const
{
	return GetItemCount(Item) >= MinCount;
}

int32 UInventoryComponent::FindEntryIndex(const UItemDataAsset* Item) const
{
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].Item == Item)
		{
			return i;
		}
	}
	return INDEX_NONE;
}
