#pragma once

#include <string>
#include <string_view>
#include <chrono>

namespace Netcode::Network {

    class Cookie {
        std::chrono::time_point<std::chrono::steady_clock> createdAt;
        int maxAge;
        std::string name;
        std::string value;
        std::string domain;
        std::string path;
        std::string cookieString;
        bool secure;
        bool httpOnly;
    public:
        Cookie();

        const std::string & GetName() const;

        const std::string & GetValue() const;

        int GetMaxAge() const;

        bool IsSessionCookie() const;
    
        bool IsExpired() const;

        bool IsValid() const;

        std::string GetCookieString() const;

        static bool Parse(std::string_view str, Cookie & c);

    };

}
