#ifndef USERS_USER_H
#define USERS_USER_H

#include "Role.h"

#include <string>

using namespace std;

namespace users {
    
    class User {
    private:
        string id;
        string name;
        string username;
        string password;
        Role role;

    public:

        User() : role(Role::Passenger) {}

        User(const string &userId, const string &fullName, 
             const string &userName, const string &pass, Role userRole)
            : id(userId), name(fullName), username(userName), 
              password(pass), role(userRole) {}

        // Getters
        const string &getId() const { return id; }
        const string &getName() const { return name; }
        const string &getUsername() const { return username; }
        const string &getPassword() const { return password; }
        
        Role getRole() const { return role; }

        // Update user role
        void setRole(Role newRole) { role = newRole; }

        // Storage Functions
        string serialize() const;
        static User deserialize(const string &line);
    };
}

#endif
