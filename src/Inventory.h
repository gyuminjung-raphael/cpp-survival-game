#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <vector>

struct Item {
    std::string name;
    int quantity;
    std::string description;
};

class Inventory {
public:
    void addItem(const std::string& name, int quantity = 1);
    void removeItem(int index);
    void removeQuantity(int index, int amount);
    void useItem(int index);
    bool hasItem(const std::string& name, int qty = 1) const;
    void consumeItem(const std::string& name, int qty);

    void sortItems();

    bool empty() const;
    int size() const;
    const Item& getItem(int index) const;

private:
    std::vector<Item> items;
};

#endif