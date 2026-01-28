#include "User.h"
#include <sstream>

using namespace std;

namespace users {
    string User::serialize() const {

        // Format: id|name|username|password|role
        return id + "|" + name + "|" + username + "|" + password + 
               "|" + to_string(static_cast<int>(role));
    }

    User User::deserialize(const string &line) {
        stringstream ss(line);
        string id, name, username, password, roleStr;
        
        // Parse pipe-delimited values
        getline(ss, id, '|');
        getline(ss, name, '|');
        getline(ss, username, '|');
        getline(ss, password, '|');
        getline(ss, roleStr, '|');
        
        // Convert role string to enum, default to Passenger on error
        int roleValue = 0;
        try {
            roleValue = stoi(roleStr);
        } catch (...) {
            roleValue = 0;
        }
        
        return User(id, name, username, password, static_cast<Role>(roleValue));
    }
}
