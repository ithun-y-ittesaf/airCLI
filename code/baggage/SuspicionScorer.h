#ifndef BAGGAGE_SUSPICIONSCORER_H
#define BAGGAGE_SUSPICIONSCORER_H

#include <string>
using namespace std;

namespace baggage {

    class SuspicionScorer {
    public:
        // Calculate suspicion score for a passenger.
        //
        // totalWeightGrams   : actual bag weight
        // expectedWeightGrams: sum of item midpoints
        // country            : from user profile
        // lastBankName       : last bank used (empty = no bank = worst case)
        // bankScoreAdjustment: score delta from bank tier (-10, 0, or +20)
        // hasPrivateItems    : true if bag has any private items
        static int calculate(int               totalWeightGrams,
                             int               expectedWeightGrams,
                             const string &country,
                             int               bankScoreAdjustment,
                             bool              hasPrivateItems);

        // 0-24 = "LOW" | 25-49 = "MEDIUM" | 50+ = "HIGH"
        static string scoreLabel(int score);
    };

} // namespace baggage

#endif // BAGGAGE_SUSPICIONSCORER_H
