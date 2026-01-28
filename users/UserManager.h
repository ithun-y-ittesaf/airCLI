#ifndef USERS_USERMANAGER_H
#define USERS_USERMANAGER_H

#include <map>
#include <vector>
#include "User.h"

using namespace std;

namespace users {
    
    class UserManager {
    private:
        map<string, User> usersById;
        map<string, string> usernameToId;

    public:

        bool load();
        
        bool save() const;
        
        string createUser(const string &name, const string &username, 
                           const string &password);
        
        // Get a user by their ID
        const User* get(const string &id) const;
        
        // Find a user by their username
        const User* findByUsername(const string &username) const;
        
        // Verify credentials and return user ID if valid
        bool authenticate(const string &username, const string &password, 
                         string &outId) const;
        
        // Get all users
        vector<User> all() const;
        
        // Update a user's role
        bool updateRole(const string &id, Role newRole);
    };
}

#endif
