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

export module LightstreamerClient;
namespace Lightstreamer::Cpp::LightstreamerClient {
    typedef int Cookies; // TODO: Define Cookies type
    typedef int Listener; // TODO: Define Listener type
    typedef int Message; // TODO: Define Message type
    typedef int Sequence; // TODO: Define Sequence type
    typedef int DelayTimeout; // TODO: Define DelayTimeout type
    typedef int EnqueueWhileDisconnected; // TODO: Define EnqueueWhileDisconnected type
    typedef int Provider; // TODO: Define Provider type
    typedef int Factory; // TODO: Define Factory type
    typedef int Subscription; // TODO: Define Subscription type

    class LightstreamerClient {
    public:

        void addCookies(Cookies cookies) {}
        void addListener(Listener listener) {}
        void connect() {}
        void disconnect() {}
        void getCookies() {}
        void getListeners() {}
        void getStatus() {}
        void getSubscriptions() {}
        void removeListener(Listener listener) {}
        void sendMessage(Message message,Sequence sequence,DelayTimeout delaytimeout,Listener listener,EnqueueWhileDisconnected enqueuewhiledisconnected) {}
        void setLoggerProvider( Provider provider) {}
        void setTrustManagerFactory(Factory factory) {}
        void subscribe(Subscription subscription) {}
        void unsubscribe(Subscription subscription) {}

    };

}