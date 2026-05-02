#include "InventoryWidget.h"

#include "InventoryComponent.h"
#include "InventorySlotWidget.h"
#include "ItemDataAsset.h"

#include "Components/PanelWidget.h"

void UInventoryWidget::BindToInventory(UInventoryComponent* InInventory)
{
	if (Inventory == InInventory)
	{
		return;
	}

	if (Inventory)
	{
		Inventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
	}

	Inventory = InInventory;

	if (Inventory)
	{
		Inventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::HandleInventoryChanged);
	}

	RebuildSlots();
}

void UInventoryWidget::NativeDestruct()
{
	if (Inventory)
	{
		Inventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
		Inventory = nullptr;
	}
	Super::NativeDestruct();
}

void UInventoryWidget::HandleInventoryChanged(UItemDataAsset* /*Item*/, int32 /*NewCount*/)
{
	// Simple strategy: rebuild every change. Inventories are small (≤ a few
	// dozen entries even late game) so the cost is negligible and the code
	// stays trivial. Switch to incremental updates only if profiling demands it.
	RebuildSlots();
}

void UInventoryWidget::ClearSlots()
{
	if (SlotsContainer)
	{
		SlotsContainer->ClearChildren();
	}
	SpawnedSlots.Reset();
}

void UInventoryWidget::RebuildSlots()
{
	ClearSlots();

	if (!Inventory || !SlotsContainer || !SlotWidgetClass)
	{
		OnInventoryRebuilt(0);
		return;
	}

	const TArray<FInventoryEntry>& Entries = Inventory->GetEntries();
	SpawnedSlots.Reserve(Entries.Num());

	for (const FInventoryEntry& Entry : Entries)
	{
		if (!Entry.Item || Entry.Count <= 0)
		{
			continue;
		}

		UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
		if (!SlotWidget)
		{
			continue;
		}

		SlotsContainer->AddChild(SlotWidget);
		SlotWidget->SetEntry(Entry.Item, Entry.Count);
		SpawnedSlots.Add(SlotWidget);
	}

	OnInventoryRebuilt(SpawnedSlots.Num());
}
