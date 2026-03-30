#include "Item.h"
#include <sstream>
using namespace std;

namespace baggage {

string Item::serialize() const {
    return id                             + "|"
         + name                           + "|"
         + category                       + "|"
         + to_string(minWeightGrams) + "|"
         + to_string(maxWeightGrams) + "|"
         + (isBanned ? "1" : "0");
}

Item Item::deserialize(const string &line) {
    stringstream ss(line);
    string id, name, cat, minStr, maxStr, banStr;
    getline(ss, id,     '|');
    getline(ss, name,   '|');
    getline(ss, cat,    '|');
    getline(ss, minStr, '|');
    getline(ss, maxStr, '|');
    getline(ss, banStr, '|');

    int  minW = 0, maxW = 0;
    bool ban  = false;
    try { minW = stoi(minStr); } catch (...) {}
    try { maxW = stoi(maxStr); } catch (...) {}
    try { ban  = (stoi(banStr) == 1); } catch (...) {}

    return Item(id, name, cat, minW, maxW, ban);
}

} // namespace baggage
