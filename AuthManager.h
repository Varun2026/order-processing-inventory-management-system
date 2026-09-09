#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <string>
#include "User.h"

class DatabaseDAO; // forward declaration

class AuthManager {
public:
    explicit AuthManager(DatabaseDAO& dao);

    // Prompts for username + password (password hidden while typing),
    // checks against the database, returns a valid User on success.
    // Returns User with id == -1 on failure.
    User login(int maxAttempts);

    // Hashes a plaintext password with SHA-256, returns lowercase hex digest.
    static std::string hashPassword(const std::string& plaintext);

private:
    DatabaseDAO& dao_;
    static std::string readHiddenInput(const std::string& prompt);
};

#endif // AUTH_MANAGER_H
