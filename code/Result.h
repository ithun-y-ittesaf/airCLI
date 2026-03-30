#ifndef RESULT_H
#define RESULT_H

#include <string>
using namespace std;

struct Result {
    bool        ok;
    string message;

    static Result Ok(const string &msg = "") { return {true,  msg}; }
    static Result Err(const string &msg)     { return {false, msg}; }
};

#endif // RESULT_H
