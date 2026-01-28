#include <string>

using namespace std;

struct Result {
        bool ok;            
        string message; 

        static Result Ok(const string &msg = "") { 
            return {true, msg}; 
        }

        // Factory method for error results
        static Result Err(const string &msg) { 
            return {false, msg}; 
        }
    };