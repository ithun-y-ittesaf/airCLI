#include "DataStorage.h"
#include <fstream>

using namespace std;

namespace storage {
    string DataStorage::pathForDataFile(const string &filename) {
        return "data/" + filename;
    }

    vector <string> DataStorage::readAll(const string &filename) {
        vector<string> lines;
        ifstream file(pathForDataFile(filename));
        string line;
        while (getline(file, line)) {
            lines.push_back(line);
        }
        return lines;
    }

    bool DataStorage::writeAll(const string &filename, const vector<string> &lines) {
        ofstream file(pathForDataFile(filename), ios::trunc);
        if (!file.is_open()) {
            return false;
        }
        for (const auto &line : lines) {
            file << line << "\n";
        }
        return true;
    }

    bool DataStorage::appendLine(const string &filename, const string &line) {
        ofstream file(pathForDataFile(filename), ios::app);
        if (!file.is_open()) {
            return false;
        }
        file << line << "\n";
        return true;
    }
}