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
    Date: 7/3/24
 ******************************************************************************/

#ifndef LIGHTSTREAMERCLIENT_HPP
#define LIGHTSTREAMERCLIENT_HPP

#include <iostream>
#include <memory>
#include <vector>
#include <regex>
#include <mutex>
#include <thread>
#include <future>
#include <map>
#include <functional>


#include <lightstreamer/client/ConnectionDetails.hpp>
#include <lightstreamer/client/ClientListener.hpp>
#include <lightstreamer/client/events/Event.hpp>
#include <Logger.hpp>
#include <lightstreamer/client/Constants.hpp>
#include <lightstreamer/client/events/EventDispatcher.hpp>
#include <lightstreamer/client/events/ClientListenerStartEvent.hpp>
#include <lightstreamer/client/events/ClientListenerEndEvent.hpp>

// TODO: subclasses and methods

namespace lightstreamer::client {
    using namespace events;

    class LightstreamerClient {
    private:

        bool instanceFieldsInitialized = false;
        std::mutex mutex_;
        EventDispatcher<ClientListener> dispatcher = EventDispatcher<ClientListener>( std::make_unique<events::EventsThread> () ); // TODO: ClientListener no properly created


        void initializeInstanceFields() {
            if (!instanceFieldsInitialized) {
                instanceFieldsInitialized = true;
            }
        }

    public:
        std::string LIB_NAME = Constants::LIB_NAME;
        std::string LIB_VERSION = Constants::LIB_VERSION;

        static std::regex ext_alpha_numeric;

        LightstreamerClient(const std::string &serverAddress, const std::string &adapterSet) {
            initializeInstanceFields();
            // TODO: Inicialización específica y configuraciones
        }

        void addListener(ClientListener *listener) {
            std::lock_guard<std::mutex> lock(mutex_);
            dispatcher.addListener(listener, new ClientListenerStartEvent(this)); // TODO: turn this into a shared pointer



        }

        void removeListener(ClientListener *listener) {
            std::lock_guard<std::mutex> lock(mutex_);
            dispatcher.removeListener(listener, new ClientListenerEndEvent(this)); // TODO: turn this into a shared pointer
        }

        // Otros métodos públicos adaptados de la clase original en C#



    };


// Funciones y adaptaciones adicionales según sea necesario
}

#endif //LIGHTSTREAMERCLIENT_HPP
