/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 20/4/24
 ******************************************************************************/

/*******************************************************************************
 Copyright (c) 2024.

 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation, either version 3 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program. If not, see <https://www.gnu.org/licenses/>..
 ******************************************************************************/

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_HTTPCOOKIE_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_HTTPCOOKIE_HPP

#include <unordered_map>
#include <string>
#include <ctime>
#include <stdexcept>

namespace lightstreamer::client::transport::providers {


    struct HttpContext {
        std::unordered_map<std::string, std::string> requestCookies;
        std::unordered_map<std::string, std::string> responseCookies;
    };

    struct CookieOptions {
        std::time_t expires;
    };

    class IDataProtector {
    public:
        virtual std::string protect(const std::string &data) = 0;

        virtual std::string unprotect(const std::string &data) = 0;

        virtual ~IDataProtector() = default;
    };


    class HttpCookie {
    private:
        HttpContext *context;
        IDataProtector *dataProtector;
        int defaultExpireTimeInDays;

    public:
        HttpCookie(HttpContext *ctx, IDataProtector *protector, int defaultExpireDays)
                : context(ctx), dataProtector(protector), defaultExpireTimeInDays(defaultExpireDays) {}

        bool contains(const std::string &key) {
            if (!context) throw std::invalid_argument("HttpContext is null");
            return context->requestCookies.find(key) != context->requestCookies.end();
        }

        std::string get(const std::string &key) {
            if (!context) throw std::invalid_argument("HttpContext is null");
            auto it = context->requestCookies.find(key);
            if (it == context->requestCookies.end()) return "";

            std::string protectedData = it->second;
            return dataProtector->unprotect(protectedData);
        }

        void remove(const std::string &key) {
            if (!context) throw std::invalid_argument("HttpContext is null");
            context->responseCookies.erase(key);
        }

        void set(const std::string &key, const std::string &value, CookieOptions options) {
            if (!context) throw std::invalid_argument("HttpContext is null");
            std::string protectedValue = dataProtector->protect(value);
            context->responseCookies[key] = protectedValue;
        }
    };

}
#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_HTTPCOOKIE_HPP
