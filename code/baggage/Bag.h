#ifndef BAGGAGE_BAG_H
#define BAGGAGE_BAG_H

#include <string>
#include <vector>
using namespace std;

namespace baggage {

    struct PackedItem {
        string itemId;
        string itemName;
        string category;
        int         assignedWeightGrams;
        bool        isPublic;
        bool        declared;
    };

    class Bag {
    private:
        string             id;
        string             ticketId;
        string             passengerId;
        string             bagTag;
        vector<PackedItem> items;

    public:
        Bag() = default;
        Bag(const string &id,
            const string &ticketId,
            const string &passengerId,
            const string &bagTag = "")
            : id(id), ticketId(ticketId), passengerId(passengerId), bagTag(bagTag) {}

        const string             &getId()          const { return id;          }
        const string             &getTicketId()    const { return ticketId;    }
        const string             &getPassengerId() const { return passengerId; }
        const string             &getBagTag()      const { return bagTag;      }
        const vector<PackedItem> &getItems()       const { return items;       }

        void addItem(const PackedItem &item);
        int  totalWeightGrams() const;

        // Format: id|ticketId|passengerId|itemId:itemName:weight:isPublic,...
        string serialize() const;
        static Bag  deserialize(const string &line);
    };

} // namespace baggage

#endif // BAGGAGE_BAG_H
