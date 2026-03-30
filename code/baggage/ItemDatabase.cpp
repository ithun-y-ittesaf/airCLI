#include "ItemDatabase.h"
#include "../core/IdGenerator.h"
#include <algorithm>
#include <cctype>
using namespace std;

namespace baggage {

bool ItemDatabase::load() {
    itemsById.clear();
    for (const auto &line : repo.readAll("items")) {
        Item item = Item::deserialize(line);
        itemsById[item.getId()] = item;
    }
    return true;
}

bool ItemDatabase::save() const {
    vector<string> lines;
    for (const auto &p : itemsById)
        lines.push_back(p.second.serialize());
    return repo.writeAll("items", lines);
}

void ItemDatabase::addItem(const Item &item) {
    itemsById[item.getId()] = item;
    repo.appendLine("items", item.serialize());
}

bool ItemDatabase::removeItem(const string &id) {
    auto it = itemsById.find(id);
    if (it == itemsById.end()) return false;
    itemsById.erase(it);
    return save();
}

const Item *ItemDatabase::findById(const string &id) const {
    auto it = itemsById.find(id);
    return (it == itemsById.end()) ? nullptr : &it->second;
}

vector<Item> ItemDatabase::findByName(const string &keyword) const {
    auto toLower = [](string s) {
        transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    };
    string lk = toLower(keyword);
    vector<Item> results;
    for (const auto &p : itemsById) {
        if (toLower(p.second.getName()).find(lk) != string::npos)
            results.push_back(p.second);
    }
    return results;
}

vector<Item> ItemDatabase::all() const {
    vector<Item> result;
    for (const auto &p : itemsById)
        result.push_back(p.second);
    return result;
}

vector<Item> ItemDatabase::bannedItems() const {
    vector<Item> result;
    for (const auto &p : itemsById)
        if (p.second.getIsBanned())
            result.push_back(p.second);
    return result;
}

} // namespace baggage
