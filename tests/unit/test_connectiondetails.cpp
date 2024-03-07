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
#include <catch2/catch_test_macros.hpp>
#include "ConnectionDetails.hpp"

using namespace Lightstreamer::Cpp::ConnectionDetails;

TEST_CASE("ConnectionDetails Methods Test", "[connectiondetails]") {
    ConnectionDetails details= ConnectionDetailsBuilder()
                                .setUser(User("username"))
                                .setPassword(Password("password"))
                                .build();

    SECTION("getAdapterSet() Test") {
        AdapterSet set = details.getAdapterSet();
        REQUIRE(set.name == "DEFAULT");
    }

    SECTION("getClientIp() Test") {
        ClientIp ip = details.getClientIp();
        REQUIRE(ip == "SomeValues"); // replace SomeValues with the appropriate test value
    }

    SECTION("getServerAddress() Test") {
        ServerAddress address = details.getServerAddress();
        REQUIRE(address == "SomeValues"); // replace SomeValues with the appropriate test value
    }

    SECTION("getServerInstanceAddress() Test") {
        ServerInstanceAddress instanceAddress = details.getServerInstanceAddress();
        REQUIRE(instanceAddress == "SomeValues"); // replace SomeValues with the appropriate test value
    }

    SECTION("getServerSocketName() Test") {
        ServerSocketName socketName = details.getServerSocketName();
        REQUIRE(socketName == "SomeValues"); // replace SomeValues with the appropriate test value
    }

    SECTION("getSessionId() Test") {
        SessionId id = details.getSessionId();
        REQUIRE(id == "SomeValues"); // replace SomeValues with the appropriate test value
    }

    SECTION("getUser() Test") {
        User user = details.getUser();
        REQUIRE(user == "SomeValues"); // replace SomeValues with the appropriate test value
    }
}