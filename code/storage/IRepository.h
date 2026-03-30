#ifndef STORAGE_IREPOSITORY_H
#define STORAGE_IREPOSITORY_H

#include <string>
#include <vector>
using namespace std;

namespace storage {

    class IRepository {
    public:
        virtual ~IRepository() = default;
        virtual vector<string> readAll(const string &filename) = 0;
        virtual bool writeAll(const string &filename,
                              const vector<string> &lines) = 0;
        virtual bool appendLine(const string &filename,
                                const string &line) = 0;
    };

} // namespace storage

#endif // STORAGE_IREPOSITORY_H
