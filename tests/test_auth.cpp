#include <cassert>
#include <iostream>
#include <cctype>
#include "../AuthManager.h"

static int passed = 0;
static int failed = 0;

void check(bool condition, const std::string& testName) {
    if (condition) { std::cout << "  [PASS] " << testName << "\n"; ++passed; }
    else { std::cout << "  [FAIL] " << testName << "\n"; ++failed; }
}

int main() {
    std::cout << "Running AuthManager password hashing tests...\n\n";

    std::string h1 = AuthManager::hashPassword("alice123");
    std::string h2 = AuthManager::hashPassword("alice123");
    std::string h3 = AuthManager::hashPassword("different_password");

    check(h1 == h2, "Same password produces the same hash (deterministic)");
    check(h1 != h3, "Different passwords produce different hashes");
    check(h1.length() == 64, "Hash output is 64 hex characters (SHA-256)");

    bool isHex = true;
    for (char c : h1) {
        if (!std::isxdigit(static_cast<unsigned char>(c)) || std::isupper(static_cast<unsigned char>(c))) {
            isHex = false; break;
        }
    }
    check(isHex, "Hash output is lowercase hexadecimal");

    std::string knownHash = AuthManager::hashPassword("password123");
    std::string expected = "ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f";
    check(knownHash == expected, "Known-answer test: SHA-256('password123') matches expected digest");

    std::cout << "\n" << passed << " passed, " << failed << " failed.\n";
    return failed == 0 ? 0 : 1;
}
