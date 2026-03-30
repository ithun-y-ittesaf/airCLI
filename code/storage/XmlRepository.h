#ifndef STORAGE_XMLREPOSITORY_H
#define STORAGE_XMLREPOSITORY_H

#include "IRepository.h"
#include <string>
#include <vector>
using namespace std;

namespace storage {

    class XmlRepository : public IRepository {
    public:
        explicit XmlRepository(const string &dataDir = "storage/data");

        vector<string> readAll(const string &filename) override;
        bool writeAll(const string &filename,
                      const vector<string> &lines) override;
        bool appendLine(const string &filename,
                        const string &line) override;

    private:
        string dataDir;

        string fullPath(const string &filename) const;
        static string extractRecord(const string &line);
        static string wrapRecord(const string &data);
        static string xmlEscape(const string &data);
        static string xmlUnescape(const string &data);
    };

} // namespace storage

#endif // STORAGE_XMLREPOSITORY_H
