#ifndef ACCESS_ACCESSCONTROL_H
#define ACCESS_ACCESSCONTROL_H

#include <string>
#include <initializer_list>

#include "../Result.h"
#include "../Role.h"

namespace access {
    
    class AccessControl {
    public:
        
        static Result requireRole(Role currentRole, Role requiredRole);

        static bool anyOf(Role currentRole, const std::initializer_list<Role> &allowedRoles);

        static std::string roleToString(Role role);
    };
}

#endif
