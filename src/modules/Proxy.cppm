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

export module Proxy;
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

    };

}