#ifndef BAGGAGE_ITEM_H
#define BAGGAGE_ITEM_H

#include <string>
using namespace std;

namespace baggage {

    class Item {
    private:
        string id;
        string name;
        string category;
        int         minWeightGrams;
        int         maxWeightGrams;
        bool        isBanned;

    public:
        Item() : minWeightGrams(0), maxWeightGrams(0), isBanned(false) {}

        Item(const string &id, const string &name,
             const string &category,
             int minWeightGrams, int maxWeightGrams, bool isBanned)
            : id(id), name(name), category(category),
              minWeightGrams(minWeightGrams), maxWeightGrams(maxWeightGrams),
              isBanned(isBanned) {}

        const string &getId()       const { return id;             }
        const string &getName()     const { return name;           }
        const string &getCategory() const { return category;       }
        int  getMinWeightGrams()         const { return minWeightGrams; }
        int  getMaxWeightGrams()         const { return maxWeightGrams; }
        bool getIsBanned()               const { return isBanned;       }

        int midWeightGrams() const { return (minWeightGrams + maxWeightGrams) / 2; }

        // Format: id|name|category|minWeight|maxWeight|isBanned
        string serialize() const;
        static Item deserialize(const string &line);
    };

} // namespace baggage

#endif // BAGGAGE_ITEM_H
