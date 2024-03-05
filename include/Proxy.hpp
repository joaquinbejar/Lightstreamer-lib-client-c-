/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 2/3/24
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
#ifndef PROXY_HPP
#define PROXY_HPP
#include <string>

namespace Lightstreamer::Cpp::Proxy {
    /**
    Simple class representing a Proxy configuration.

    An instance of this class can be used through \ref `ConnectionOptions.setProxy` to
            instruct a LightstreamerClient to connect to the Lightstreamer Server passing through a proxy.

    @param type: the proxy type. Supported values are HTTP, SOCKS4 and SOCKS5.
    @param host: the proxy host
    @param port: the proxy port
    @param user: the user name to be used to validate against the proxy. Optional.
    @param password: the password to be used to validate against the proxy. Optional.
    **/
    class Proxy {
    public:
        std::string type;
        std::string host;
        int port;
        std::string user;
        std::string password;

        Proxy(std::string type, std::string host, int port, std::string user,
              std::string password) : type(std::move(type)),
                                      host(std::move(host)), port(port), user(std::move(user)),
                                      password(std::move(password)) {
        }

        operator std::string() const {
            if (user.empty()) {
                return type + " " + host + ":" + std::to_string(port);
            }
            return type + " " + user + "@" + host + ":" + std::to_string(port);
        }

        std::string operator()() const {
            return this->operator std::string();
        }

        bool operator==(const Proxy &other) const {
            if (this->type == other.type && this->host == other.host && this->port == other.port &&
                this->user == other.user && this->password == other.password) {
                return true;
            }
            return false;
        }
    };
}
#endif //PROXY_HPP
