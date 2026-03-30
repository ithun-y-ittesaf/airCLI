#ifndef BAGGAGE_ITEMDATABASE_H
#define BAGGAGE_ITEMDATABASE_H

#include <map>
#include <string>
#include <vector>
#include "Item.h"
#include "../storage/IRepository.h"
using namespace std;

namespace baggage {

    class ItemDatabase {
    private:
        storage::IRepository       &repo;
        map<string, Item> itemsById;

    public:
        explicit ItemDatabase(storage::IRepository &repo) : repo(repo) {}

        bool load();
        bool save() const;

        void addItem(const Item &item);
        bool removeItem(const string &id);

        const Item       *findById(const string &id) const;
        vector<Item> findByName(const string &keyword) const;
        vector<Item> all() const;
        vector<Item> bannedItems() const;
    };

} // namespace baggage

#endif // BAGGAGE_ITEMDATABASE_H
