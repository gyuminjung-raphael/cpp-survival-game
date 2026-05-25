#include "Inventory.h"
#include "Item.h"
#include <algorithm>

// Priority: 0=Material, 1=Food/Drink, 2=Healing, 3=Tools
// Sort key: lower number = listed earlier (materials < food < healing < tools).
static int itemSortPriority(const std::string& name) {
    const ItemDef* def = getItemDef(name);
    if (!def) return 99;
    switch (def->type) {
        case ITEM_MATERIAL: return 0;
        case ITEM_FOOD:
        case ITEM_DRINK:
            return (def->hpRestore > 0 || def->fatigueRestore > 0) ? 2 : 1;
        case ITEM_TOOL:
            return def->consumable ? 2 : 3;
        default: return 99;
    }
}

// Reorder the inventory list by category, then alphabetically within category.
void Inventory::sortItems() {
    std::stable_sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
        int pa = itemSortPriority(a.name);
        int pb = itemSortPriority(b.name);
        if (pa != pb) return pa < pb;
        return a.name < b.name;
    });
}

// Add an item; stacks with an existing entry of the same name if one exists.
void Inventory::addItem(const std::string& name, int quantity) {
    for (std::vector<Item>::iterator it = items.begin(); it != items.end(); ++it) {
        if (it->name == name) {
            it->quantity += quantity;
            return;
        }
    }

    Item item;
    item.name = name;
    item.quantity = quantity;
    const ItemDef* def = getItemDef(name);
    item.description = def ? def->description : "Unknown item.";
    items.push_back(item);
    sortItems();
}

// Erase an item slot completely by index.
void Inventory::removeItem(int index) {
    if (index >= 0 && index < (int)items.size()) {
        items.erase(items.begin() + index);
    }
}

// Remove `amount` from a stack at `index`; deletes the slot if it would empty.
void Inventory::removeQuantity(int index, int amount) {
    if (index < 0 || index >= (int)items.size() || amount <= 0) return;

    if (amount >= items[index].quantity) {
        items.erase(items.begin() + index);
    } else {
        items[index].quantity -= amount;
    }
}

// Placeholder hook; the actual "use" effect is applied in InventoryUI.
void Inventory::useItem(int index) {
    if (index >= 0 && index < (int)items.size()) {
        // no-op: actual use logic is handled in InventoryUI
    }
}

// True if the inventory contains at least `qty` of an item with the given name.
bool Inventory::hasItem(const std::string& name, int qty) const {
    for (const auto& item : items) {
        if (item.name == name && item.quantity >= qty) return true;
    }
    return false;
}

// Remove `qty` of an item by name; deletes the entry if depleted.
void Inventory::consumeItem(const std::string& name, int qty) {
    for (int i = 0; i < (int)items.size(); i++) {
        if (items[i].name == name) {
            items[i].quantity -= qty;
            if (items[i].quantity <= 0)
                items.erase(items.begin() + i);
            return;
        }
    }
}

// True if the inventory has no item slots.
bool Inventory::empty() const {
    return items.empty();
}

// Number of distinct item slots (not the total stack count).
int Inventory::size() const {
    return (int)items.size();
}

// Read-only access to a slot by index.
const Item& Inventory::getItem(int index) const {
    return items[index];
}