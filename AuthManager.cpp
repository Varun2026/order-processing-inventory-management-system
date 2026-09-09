#include "AuthManager.h"
#include "DatabaseDAO.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <termios.h>
#include <unistd.h>
#include <openssl/evp.h>

AuthManager::AuthManager(DatabaseDAO& dao) : dao_(dao) {}

std::string AuthManager::readHiddenInput(const std::string& prompt) {
    std::cout << prompt;
    std::cout.flush();

    termios oldt{};
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string input;
    std::getline(std::cin, input);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
    return input;
}

std::string AuthManager::hashPassword(const std::string& plaintext) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, plaintext.c_str(), plaintext.size());
    EVP_DigestFinal_ex(ctx, digest, &digestLen);
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < digestLen; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return oss.str();
}

User AuthManager::login(int maxAttempts) {
    for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
        std::cout << "\n--- Login (attempt " << attempt << "/" << maxAttempts << ") ---\n";
        std::cout << "Username: ";
        std::string username;
        std::getline(std::cin, username);

        std::string password = readHiddenInput("Password: ");
        std::string hashed = hashPassword(password);

        User user = dao_.authenticateUser(username, hashed);
        if (user.isValid()) {
            std::cout << "Login successful. Welcome, " << user.username
                      << " (" << roleToString(user.role) << ")\n";
            dao_.logAudit(user.username, "LOGIN", "SUCCESS");
            return user;
        }
        std::cout << "Invalid username or password.\n";
        dao_.logAudit(username, "LOGIN", "FAILED");
    }
    std::cout << "Too many failed login attempts.\n";
    return User{};
}
