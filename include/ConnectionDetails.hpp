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

#ifndef CONNECTIONDETAILS_HPP
#define CONNECTIONDETAILS_HPP

#include <string>


namespace Lightstreamer::Cpp::ConnectionDetails {
    typedef std::string Password; // TODO:  Define the type of Password
    typedef std::string User; // TODO:  Define the type of User
    typedef std::string ServerAddress; // TODO:  Define the type of ServerAddress
    typedef std::string ClientIp; // TODO:  Define the type of ClientIp
    typedef std::string ServerInstanceAddress; // TODO:  Define the type of ServerInstanceAddress
    typedef std::string ServerSocketName; // TODO:  Define the type of getServerSocketName
    typedef std::string SessionId; // TODO:  Define the type of getSessionId

    struct AdapterSet {
        std::string name = "DEFAULT";
    };

    class ConnectionDetailsInterface {
    protected:
        AdapterSet m_adapterSet;
        User m_user;
        Password m_password;
        ClientIp m_clientIp;
        ServerAddress m_serverAddress;
        ServerInstanceAddress m_serverInstanceAddress;
        ServerSocketName m_serverSocketName;
        SessionId m_sessionId;
    public:
        virtual void setAdapterSet(const AdapterSet &adapterset) = 0;
        virtual void setPassword(const Password &password) = 0;
        virtual void setServerAddress(const ServerAddress &serveraddress) = 0;
        virtual void setUser(const User &user) = 0;
        virtual ~ConnectionDetailsInterface() = default;
    };


    /**
    Used by LightstreamerClient to provide a basic connection properties data object.

    Data object that contains the configuration settings needed to connect to a Lightstreamer Server.

    An instance of this class is attached to every \ref `LightstreamerClient`
    as \ref `LightstreamerClient.connectionDetails`

    .. seealso:: \ref `LightstreamerClient`
    **/
    class ConnectionDetails : ConnectionDetailsInterface {

    public:
        /**
        Inquiry method that gets the name of the Adapter Set (which defines the Metadata Adapter and one or several
        Data Adapters) mounted on Lightstreamer Server that supply all the items used in this application.

        @return: the adapterSet the name of the Adapter Set; returns None if no name has been configured, that means 
         that the "DEFAULT" Adapter Set is used.

        .. seealso:: \ref `setAdapterSet`
        **/
        [[nodiscard]] AdapterSet getAdapterSet() const {
            return m_adapterSet;
        }

        /**
        Inquiry method that gets the IP address of this client as seen by the Server which is serving
        the current session as the client remote address (note that it may not correspond to the client host;
        for instance it may refer to an intermediate proxy). If, upon a new session, this address changes,
         it may be a hint that the intermediary network nodes handling the connection have changed, hence the network
        capabilities may be different. The library uses this information to optimize the connection.

        Note that in case of polling or in case rebind requests are needed, subsequent requests related to the same
        session may, in principle, expose a different IP address to the Server; these changes would not be reported.

        @b lifecycle If a session is not currently active, None is returned;
        soon after a session is established, the value may become available.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "clientIp" on any
        ClientListener listening to the related LightstreamerClient.

        @return:  A canonical representation of an IP address (it can be either IPv4 or IPv6), or None.
        **/
        [[nodiscard]] ClientIp getClientIp() const {
            return m_clientIp;
        }

        /**
        Inquiry method that gets the configured address of Lightstreamer Server.

        @return: the serverAddress the configured address of Lightstreamer Server.
        **/
        [[nodiscard]] ServerAddress getServerAddress() const {
            return m_serverAddress;
        }

        /**
        Inquiry method that gets the server address to be used to issue all requests related to the current session.
        In fact, when a Server cluster is in place, the Server address specified through \ref `setServerAddress` can
         identify various Server instances; in order to ensure that all requests related to a session are issued to
        the same Server instance, the Server can answer to the session opening request by providing an address which
        uniquely identifies its own instance. When this is the case, this address is returned by the method; otherwise,
        None is returned.

        Note that the addresses will always have the http: or https: scheme. In case WebSockets are used, the specified
        scheme is internally converted to match the related WebSocket protocol (i.e. http becomes ws while
        https becomes wss).

        general edition note** Server Clustering is an optional feature, available depending on Edition and License
        Type. To know what features are enabled by your license, please see the License tab of the Monitoring Dashboard
        (by default, available at /dashboard).

        @b lifecycle If a session is not currently active, null is returned; soon after a session is established,
        the value may become available.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "serverInstanceAddress" on any
        ClientListener listening to the related LightstreamerClient.

        @return: address used to issue all requests related to the current session, or None.
        **/
        [[nodiscard]] ServerInstanceAddress getServerInstanceAddress() const {
            return m_serverInstanceAddress;
        }

        /**
        Inquiry method that gets the instance name of the Server which is serving the current session. To be more
        precise, each answering port configured on a Server instance (through a <http_server> or <https_server> element
        in the Server configuration file) can be given a different name; the name related to the port to which the
        session opening request has been issued is returned.

        Note that each rebind to the same session can, potentially, reach the Server on a port different than the one
        used for the previous request, depending on the behavior of intermediate nodes. However, the only meaningful
        case is when a Server cluster is in place and it is configured in such a way that the port used for all
        bind_session requests differs from the port used for the initial create_session request.

        @b "general edition note" Server Clustering is an optional feature, available depending on Edition and License Type.
        To know what features are enabled by your license, please see the License tab of the Monitoring Dashboard
        (by default, available at /dashboard).

        @b lifecycle If a session is not currently active, None is returned;
        soon after a session is established, the value will become available.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "serverSocketName" on any
        ClientListener listening to the related LightstreamerClient.

        @return: name configured for the Server instance which is managing the current session, or None.
        **/
        [[nodiscard]] ServerSocketName getServerSocketName() const {
            return m_serverSocketName;
        }

        /**
        Inquiry method that gets the ID associated by the server to this client session.

        @b lifecycle If a session is not currently active, null is returned; soon after a session is established,
        the value will become available.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "sessionId" on any
        ClientListener listening to the related LightstreamerClient.

        @return: ID assigned by the Server to this client session, or None.
        **/
        [[nodiscard]] SessionId getSessionId() const {
            return m_sessionId;
        }

        /**
        Inquiry method that gets the username to be used for the authentication on Lightstreamer Server when
        initiating the session.

        @return: the username to be used for the authentication on Lightstreamer Server; returns None if no user name
         has been configured.
        **/
        [[nodiscard]] User getUser() const {
            return m_user;
        }

        /**
        Setter method that sets the name of the Adapter Set mounted on Lightstreamer Server to be used to handle
        all requests in the session.

        An Adapter Set defines the Metadata Adapter and one or several Data Adapters. It is configured on the
                server side through an "adapters.xml" file; the name is configured through the "id" attribute in
        the <adapters_conf> element.

        @b default The default Adapter Set, configured as "DEFAULT" on the Server.

        @b lifecycle The Adapter Set name should be set on the \ref `LightstreamerClient.connectionDetails` object
        before calling the \ref `LightstreamerClient.connect` method. However, the value can be changed at any time:
        the supplied value will be used for the next time a new session is requested to the server.

        This setting can also be specified in the \ref `LightstreamerClient` constructor.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "adapterSet" on any
        ClientListener listening to the related LightstreamerClient.

        @param adapterset: The name of the Adapter Set to be used. A None value is equivalent to the "DEFAULT" name.
        **/
        void setAdapterSet(const AdapterSet &adapterset) override {
            m_adapterSet = adapterset;
        }

        /**
        Setter method that sets the password to be used for the authentication on Lightstreamer Server when initiating
        the session. The Metadata Adapter is responsible for checking the credentials (username and password).

        @b default  If no password is supplied, no password information will be sent at session initiation.
        The Metadata Adapter, however, may still allow the session.

        @b lifecycle The username should be set on the \ref `LightstreamerClient.connectionDetails` object before calling
        the \ref `LightstreamerClient.connect` method. However, the value can be changed at any time: the supplied
        value will be used for the next time a new session is requested to the server.

        NOTE: The password string will be stored in the current instance. That is necessary in order to allow
        automatic reconnection/reauthentication for fail-over. For maximum security, avoid using an actual private
        password to authenticate on Lightstreamer Server; rather use a session-id originated by your web/application
        server, that can be checked by your Metadata Adapter.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "password" on any
        ClientListener listening to the related LightstreamerClient.

        @param password: The password to be used for the authentication on Lightstreamer Server.
        The password can be None.

        .. seealso:: \ref `setUser`
        **/
        void setPassword(const Password &password) override {
            m_password = password;
        }

        /**
        Setter method that sets the address of Lightstreamer Server.

        Note that the addresses specified must always have the http: or https: scheme. In case WebSockets are used,
        the specified scheme is internally converted to match the related WebSocket protocol (i.e. http becomes ws
        while https becomes wss).

        @b "general edition note" WSS/HTTPS is an optional feature, available depending on Edition and License Type.
        To know what features are enabled by your license, please see the License tab of the Monitoring Dashboard (by default,
        available at /dashboard).

        @b default if no server address is supplied the client will be unable to connect.

        @b lifecycle This method can be called at any time. If called while connected, it will be applied when the next
        session creation request is issued. This setting can also be specified in the \ref `LightstreamerClient`
        constructor.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "serverAddress" on any
        ClientListener listening to the related LightstreamerClient.

        @param serveraddress: The full address of Lightstreamer Server. A None value can also be used, to restore the default value.

        An IPv4 or IPv6 can also be used in place of a hostname. Some examples of valid values include: ::

        http://push.mycompany.com
        http://push.mycompany.com:8080
                http://79.125.7.252
        http://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]
                http://[2001:0db8:85a3::8a2e:0370:7334]:8080

        @throw IllegalArgumentException: if the given address is not valid.
        **/
        void setServerAddress(const ServerAddress &serveraddress) override {
            // TODO: check if the address is valid
            m_serverAddress = serveraddress;
        }

        /**
        Setter method that sets the username to be used for the authentication on Lightstreamer Server when initiating
        the session. The Metadata Adapter is responsible for checking the credentials (username and password).

        @b default If no username is supplied, no user information will be sent at session initiation.
        The Metadata Adapter, however, may still allow the session.

        @b lifecycle The username should be set on the \ref `LightstreamerClient.connectionDetails` object before
        calling the \ref `LightstreamerClient.connect` method. However, the value can be changed at any time: the
        supplied value will be used for the next time a new session is requested to the server.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "user" on any
        ClientListener listening to the related LightstreamerClient.

        @param user: The username to be used for the authentication on Lightstreamer Server. The username can be None.

        .. seealso:: \ref `setPassword`
        **/
        void setUser(const User &user) override {
            m_user = user;
        }
    };

    class ConnectionDetailsBuilder {
    private:
        ConnectionDetails m_details;
    public:
        ConnectionDetailsBuilder& setAdapterSet(const AdapterSet &adapterset) {
            m_details.setAdapterSet(adapterset);
            return *this;
        }
        ConnectionDetailsBuilder& setPassword(const Password &password) {
            m_details.setPassword(password);
            return *this;
        }
        ConnectionDetailsBuilder& setServerAddress(const ServerAddress &serveraddress) {
            m_details.setServerAddress(serveraddress);
            return *this;
        }
        ConnectionDetailsBuilder& setUser(const User &user) {
            m_details.setUser(user);
            return *this;
        }

        ConnectionDetails build() {return m_details;}
    };
}

#endif //CONNECTIONDETAILS_HPP
