#include "UserManager.h"
#include "../storage/DataStorage.h"
#include <chrono>

using namespace std;

namespace users {
    bool UserManager::load() {
        auto lines = storage::DataStorage::readAll("users.txt");
        usersById.clear();
        usernameToId.clear();
        for (const auto &l : lines) {
            User u = User::deserialize(l);
            usersById[u.getId()] = u;
            usernameToId[u.getUsername()] = u.getId();
        }
        return true;
    }

    bool UserManager::save() const {
        std::vector<std::string> lines;
        for (const auto &p : usersById) {
            lines.push_back(p.second.serialize());
        }
        return storage::DataStorage::writeAll("users.txt", lines);
    }

    string UserManager::createUser(const std::string &name, const std::string &username, const std::string &password) {
        using namespace std::chrono;
        auto t = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        string id = "U" + std::to_string(t);
        while (usersById.find(id) != usersById.end()) {
            ++t; id = "U" + std::to_string(t);
        }
        User u(id, name, username, password, Role::Passenger);
        usersById[id] = u;
        usernameToId[username] = id;
        storage::DataStorage::appendLine("users.txt", u.serialize());
        return id;
    }

    const User* UserManager::get(const string &id) const {
        auto it = usersById.find(id);
        if (it == usersById.end()) return nullptr;
        return &it->second;
    }

    const User* UserManager::findByUsername(const std::string &username) const {
        auto it = usernameToId.find(username);
        if (it == usernameToId.end()) return nullptr;
        return get(it->second);
    }

    bool UserManager::authenticate(const std::string &username, const std::string &password, string &outId) const {
        const User* user = findByUsername(username);
        if (!user) return false;
        if (user->getPassword() != password) return false;
        outId = user->getId();
        return true;
    }

    std::vector<User> UserManager::all() const {
        std::vector<User> v;
        v.reserve(usersById.size());
        for (const auto &p : usersById) v.push_back(p.second);
        return v;
    }

    bool UserManager::updateRole(const string &id, Role newRole) {
        auto it = usersById.find(id);
        if (it == usersById.end()) return false;
        it->second.setRole(newRole);
        return save();
    }
}
