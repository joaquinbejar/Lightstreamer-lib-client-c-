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

/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 5/3/24
 ******************************************************************************/

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <Proxy.hpp>

using namespace Lightstreamer::Cpp::Proxy;

TEST_CASE("Proxy class tests", "[proxy]") {
    SECTION("Constructor and getter tests") {
        std::string type = "HTTP";
        std::string host = "localhost";
        int port = 8080;
        std::string user = "user";
        std::string password = "password";

        Proxy proxy(type, host, port, user, password);

        REQUIRE(proxy.type == type);
        REQUIRE(proxy.host == host);
        REQUIRE(proxy.port == port);
        REQUIRE(proxy.user == user);
        REQUIRE(proxy.password == password);
    }

    SECTION("Constructor by list") {
        std::string host = "localhost";
        int port = 8080;
        std::string user = "user";
        std::string password = "password";

        Proxy proxy = {"HTTP", host, port, user, password};

        REQUIRE(proxy.type == "HTTP");
        REQUIRE(proxy.host == host);
        REQUIRE(proxy.port == port);
        REQUIRE(proxy.user == user);
        REQUIRE(proxy.password == password);
    }


    SECTION("Equality operator tests") {
        std::string type = "HTTP";
        std::string host = "localhost";
        int port = 8080;
        std::string user = "user";
        std::string password = "password";

        Proxy proxy1(type, host, port, user, password);
        Proxy proxy2(type, host, port, user, password);
        Proxy proxy3("SOCKS5", host, port, user, password);

        REQUIRE(proxy1 == proxy2);
        REQUIRE_FALSE(proxy1 == proxy3);
    }

    SECTION("String operator tests") {
        std::string type = "HTTP";
        std::string host = "localhost";
        int port = 8080;
        std::string user = "user";
        std::string password = "password";

        Proxy proxy1(type, host, port, user, password);
        Proxy proxy2(type, host, port, "", "");

        REQUIRE(static_cast<std::string>(proxy1) == "HTTP user@localhost:8080");
        REQUIRE(static_cast<std::string>(proxy2) == "HTTP localhost:8080");

        REQUIRE(proxy1() == "HTTP user@localhost:8080");
        REQUIRE(proxy2() == "HTTP localhost:8080");
    }
}
