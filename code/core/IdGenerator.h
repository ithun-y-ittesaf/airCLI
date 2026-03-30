#ifndef CORE_IDGENERATOR_H
#define CORE_IDGENERATOR_H

#include <string>
#include <functional>
using namespace std;

namespace core {

    class IdGenerator {
    public:
        static string next(
            const string &prefix,
            const function<bool(const string &)> &isUnique
        );
    };

} // namespace core

#endif // CORE_IDGENERATOR_H
