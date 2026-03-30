#include "XmlRepository.h"
#include <fstream>
#include <filesystem>
using namespace std;

namespace storage {

XmlRepository::XmlRepository(const string &dir) : dataDir(dir) {}

vector<string> XmlRepository::readAll(const string &filename) {
    vector<string> records;
    ifstream file(fullPath(filename));
    if (!file.is_open()) return records;
    string line;
    while (getline(file, line)) {
        string record = extractRecord(line);
        if (!record.empty())
            records.push_back(xmlUnescape(record));
    }
    return records;
}

bool XmlRepository::writeAll(const string &filename,
                              const vector<string> &lines) {
    filesystem::create_directories(dataDir);
    ofstream file(fullPath(filename), ios::trunc);
    if (!file.is_open()) return false;
    file << "<records>\n";
    for (const auto &line : lines)
        file << "  " << wrapRecord(xmlEscape(line)) << "\n";
    file << "</records>\n";
    return true;
}

bool XmlRepository::appendLine(const string &filename,
                                const string &line) {
    string path = fullPath(filename);
    if (!filesystem::exists(path))
        return writeAll(filename, {line});

    ifstream inFile(path);
    if (!inFile.is_open()) return false;
    vector<string> rawLines;
    string rawLine;
    while (getline(inFile, rawLine))
        rawLines.push_back(rawLine);
    inFile.close();

    if (!rawLines.empty() &&
        rawLines.back().find("</records>") != string::npos)
        rawLines.pop_back();

    rawLines.push_back("  " + wrapRecord(xmlEscape(line)));
    rawLines.push_back("</records>");

    ofstream outFile(path, ios::trunc);
    if (!outFile.is_open()) return false;
    for (const auto &l : rawLines)
        outFile << l << "\n";
    return true;
}

string XmlRepository::fullPath(const string &filename) const {
    string name = filename;
    if (name.size() < 4 || name.substr(name.size() - 4) != ".xml")
        name += ".xml";
    return dataDir + "/" + name;
}

string XmlRepository::extractRecord(const string &line) {
    const string open  = "<record>";
    const string close = "</record>";
    auto start = line.find(open);
    auto end   = line.find(close);
    if (start == string::npos || end == string::npos) return "";
    start += open.size();
    if (start >= end) return "";
    return line.substr(start, end - start);
}

string XmlRepository::wrapRecord(const string &data) {
    return "<record>" + data + "</record>";
}

string XmlRepository::xmlEscape(const string &data) {
    string result;
    result.reserve(data.size());
    for (char c : data) {
        switch (c) {
            case '&':  result += "&amp;";  break;
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '"':  result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default:   result += c;        break;
        }
    }
    return result;
}

string XmlRepository::xmlUnescape(const string &data) {
    string result;
    result.reserve(data.size());
    size_t i = 0;
    while (i < data.size()) {
        if (data[i] == '&') {
            auto semi = data.find(';', i);
            if (semi != string::npos) {
                string entity = data.substr(i, semi - i + 1);
                if      (entity == "&amp;")  { result += '&';  i = semi + 1; continue; }
                else if (entity == "&lt;")   { result += '<';  i = semi + 1; continue; }
                else if (entity == "&gt;")   { result += '>';  i = semi + 1; continue; }
                else if (entity == "&quot;") { result += '"';  i = semi + 1; continue; }
                else if (entity == "&apos;") { result += '\''; i = semi + 1; continue; }
            }
        }
        result += data[i++];
    }
    return result;
}

} // namespace storage
