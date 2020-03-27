#include "Cookie.h"
#include <boost/algorithm/string.hpp>
#include <vector>
#include <algorithm>

Cookie::Cookie() : name{}, value{}, maxAge{}, domain{}, secure{}, httpOnly{}, createdAt{} { }

const std::string & Cookie::GetName() const {
	return name;
}

const std::string & Cookie::GetValue() const {
    return value;
}

int Cookie::GetMaxAge() const {
    return maxAge;
}

bool Cookie::IsSessionCookie() const {
    return maxAge == 0;
}

bool Cookie::IsExpired() const {
    return ((createdAt + std::chrono::seconds(maxAge)) < std::chrono::steady_clock::now());
}

bool Cookie::IsValid() const
{
    return !name.empty() && !value.empty();
}

std::string Cookie::GetCookieString() const {
    return cookieString;
}

bool Cookie::Parse(std::string_view str, Cookie & c) {
    enum {
        EXPECTING_EQUALS = 1, EXPECTING_SEMI = 2, ERROR = 0
    };

    Cookie cookie;

    int state = EXPECTING_EQUALS;

    int numSemis = 0;
    size_t startIndex = 0;
    size_t countOf = 0;
    std::string key;
    std::string val;
    for(size_t i = 0; (i < str.size()); ++i) {

        if(str[i] == '=') {

            if((state & EXPECTING_EQUALS) > 0) {

                key = str.substr(startIndex, countOf);
                state = EXPECTING_SEMI;
                startIndex = i + 1;
                countOf = 0;

            } else {
                state = ERROR;
            }

        } else if(str[i] == ';' || (i + 1) == str.size()) {

            if((state & EXPECTING_SEMI) > 0) {

                if((i + 1) == str.size()) {
                    countOf += 1;
                }

                val = str.substr(startIndex, countOf);
                state |= EXPECTING_EQUALS;
                startIndex = i + 1;
                countOf = 0;

                boost::algorithm::trim(key);
                boost::algorithm::trim(val);
                if(numSemis == 0) {
                    cookie.name = std::move(key);
                    cookie.value = std::move(val);
                } else {
                    std::transform(std::begin(key), std::end(key), std::begin(key), [](char c) -> char { return std::tolower(c); });

                    if(key == "path") {
                        cookie.path = std::move(val);
                    } else if(key == "max-age") {
                        cookie.maxAge = std::stoi(val);
                        if(val != std::to_string(cookie.maxAge)) {
                            state = ERROR;
                        }
                    } else if(key == "expires") {
                        // maybe
                    } else if(key == "path") {
                        cookie.path = std::move(val);
                    } else if(key == "domain") {
                        cookie.domain = std::move(val);
                    } else if(key == "secure" && val.empty()) {
                        cookie.secure = true;
                    } else if(key == "httponly" && val.empty()) {
                        cookie.httpOnly = true;
                    }
                }

                ++numSemis;
                key.clear();
                val.clear();

            } else {
                state = ERROR;
            }

        } else countOf += 1;

    }

    if(state == ERROR) {
        return false;
    }

    cookie.createdAt = std::chrono::steady_clock::now();
    cookie.cookieString = cookie.name + "=" + cookie.value;
    c = std::move(cookie);


    return true;
}
