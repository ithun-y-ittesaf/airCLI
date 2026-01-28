#ifndef STORAGE_DATA_DATASTORAGE_H
#define STORAGE_DATA_DATASTORAGE_H

#include <string> 
#include <vector>

using namespace std;

namespace storage {
    class DataStorage {
    public: 
        static vector <string> readAll (const string &filename);
        static bool writeAll (const string &filename, const vector <string> &lines);
        static bool appendLine (const string &filename, const string &line);
        static string pathForDataFile (const string &filename);
    };


}

#endif