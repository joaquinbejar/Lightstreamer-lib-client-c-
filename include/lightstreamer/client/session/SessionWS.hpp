/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 22/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONWS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONWS_HPP

#include "Logger.hpp" // Assuming the existence of a similar logging system
#include <lightstreamer/client/session/Session.hpp>
#include <memory>
#include <functional>
#include <cassert>
#include <list>
#include <future>

namespace lightstreamer::client::session {

/**
 * Handles the WebSocket (WS) session part of a Lightstreamer client session.
 */
    class SessionWS : public Session {
    private:
        bool earlyOpen;

        enum class WsState {
            WS_NOT_CONNECTED,
            WS_CONNECTING,
            WS_CONNECTED,
            WS_BROKEN
        };

        class StateMachine {
        public:
            SessionWS &outerInstance;

            WsState state = WsState::WS_NOT_CONNECTED;
            std::string controlLink;
            std::future<void> openWsFuture; // Assuming the use of std::future for async operations

            StateMachine(SessionWS &outerInstance) : outerInstance(outerInstance) {}

            void createSent() {
                switch (state) {
                    case WsState::WS_NOT_CONNECTED:
                        if (outerInstance.earlyOpen) {
                            next(WsState::WS_CONNECTING, "createSent");
                            assert(controlLink.empty());
                            openWS();
                        }
                        break;

                    default:
                        assert(false); // Should not happen
                        break;
                }
            }

            std::future<void> sendBind(const std::string &bindCause) {
                switch (state) {
                    case WsState::WS_NOT_CONNECTED:
                        next(WsState::WS_CONNECTING, "sendBind");
                        openWS();
                        return outerInstance.bindSessionExecution(bindCause);

                    case WsState::WS_BROKEN:
                        next(WsState::WS_BROKEN, "sendBind");
                        // Supongamos que handler.streamSenseSwitch es una operación que no retorna un futuro
                        outerInstance.handler.streamSenseSwitch(outerInstance.handlerPhase, "ws.error",
                                                                outerInstance.phase,
                                                                outerInstance.recoveryBean.isRecovery());
                        // Aquí necesitas retornar un futuro rechazado. Debes implementar un mecanismo para ello en C++.
                        return std::async(std::launch::async,
                                          []() -> void { throw std::runtime_error("WebSocket connection broken."); });

                    default:
                        assert(state == WsState::WS_CONNECTED || state == WsState::WS_CONNECTING);
                        next(state, "sendBind");
                        return outerInstance.bindSessionExecution(bindCause);
                }
            }

            void changeControlLink(const std::string &newControlLink) {
                switch (state) {
                    case WsState::WS_NOT_CONNECTED:
                        assert(!earlyOpen); // Verifica que no se suponía que WS se abriera temprano
                        controlLink = newControlLink; // Actualiza el enlace de control
                        // No necesitas cambiar el estado o reiniciar la conexión porque WS no está conectado
                        break;

                    case WsState::WS_CONNECTING:
                    case WsState::WS_CONNECTED:
                    case WsState::WS_BROKEN:
                        assert(openWsFuture.valid()); // Asegura que hay una operación de conexión en curso
                        controlLink = newControlLink; // Actualiza el enlace de control
                        abortOpenWsFuture(); // Aborta la conexión WS pendiente o en curso
                        next(WsState::WS_CONNECTING, "clink"); // Cambia el estado a CONNECTING
                        openWS(); // Intenta abrir una nueva conexión WS con el nuevo enlace de control
                        break;
                }
            }

            void next(WsState nextState, const std::string &event) {
                if (log && log->IsDebugEnabled()) {
                    std::cout << "SessionWS state change (" << this << ") (" << event << "): "
                              << static_cast<int>(state)
                              << (state != nextState ? " -> " + std::to_string(static_cast<int>(nextState)) : "")
                              << std::endl;
                }
                state = nextState;
            }

            void connectionOK() {
                assert(state == WsState::WS_CONNECTING);
                next(WsState::WS_CONNECTED, "ok");
                // Aquí podrías querer llamar a sendBind("loop1"), dependiendo de tu lógica específica
            }

            void connectionError() {
                assert(state == WsState::WS_CONNECTING);
                next(WsState::WS_BROKEN, "error");
                if (outerInstance.isOff() || outerInstance.isCreating() || outerInstance.isStalled() ||
                    outerInstance.isCreated()) {
                    // Este es un error en una apertura temprana, no se puede actuar ahora ya que debemos esperar el bucle desde la creación
                    // De lo contrario, desperdiciaríamos la sesión entera
                    // ¡No hacer nada!
                } else {
                    launchTimeout("zeroDelay", 0, "ws.broken.wait", false);
                }
            }

            void openWS() {
                std::string cLink = controlLink.empty() ? PushServerAddress : controlLink;
                assert(!openWsFuture.valid() || openWsFuture.wait_for(std::chrono::seconds(0)) ==
                                                std::future_status::ready); // Asegura que no hay una operación pendiente

                // Aquí debes reemplazar el siguiente comentario con tu implementación para abrir una conexión WebSocket
                // Y manejar los resultados de éxito o error adecuadamente, posiblemente usando std::promise y std::future
                // openWsFuture = /* tu lógica para abrir un WebSocket y manejar el resultado */;
            }

        private:
        }

        void simulateDelay() {
            // For testing and simulation purposes
        }

    StateMachine wsMachine;

    public:
        class MyRunnableConnectOK {
            std::weak_ptr<StateMachine> ref;

        public:
            MyRunnableConnectOK(std::shared_ptr<StateMachine> sm) : ref(sm) {}

            void operator()() {
                if (auto sm = ref.lock()) { // Intenta convertir la referencia débil en una fuerte. Si el objeto aún existe, sm será válido.
                    sm->connectionOK();
                }
            }
        };

        class MyRunnableError {
            std::weak_ptr<StateMachine> ref;

        public:
            MyRunnableError(std::shared_ptr<StateMachine> sm) : ref(sm) {}

            void operator()() {
                if (auto sm = ref.lock()) { // Similar a MyRunnableConnectOK, pero para el caso de error.
                    sm->connectionError();
                }
            }
        };

        SessionWS(int objectId, bool isPolling, bool forced, std::shared_ptr<SessionListener> handler, std::shared_ptr<SubscriptionsListener> subscriptions, std::shared_ptr<MessagesListener> messages, std::shared_ptr<Session> originalSession, std::shared_ptr<SessionThread> thread, std::shared_ptr<Protocol> protocol, std::shared_ptr<InternalConnectionDetails> details, std::shared_ptr<InternalConnectionOptions> options, int callerPhase, bool retryAgainIfStreamFails, bool sessionRecovery)
                : Session(objectId, isPolling, forced, handler, subscriptions, messages, originalSession, thread, protocol, details, options, callerPhase, retryAgainIfStreamFails, sessionRecovery),
                  earlyOpen(options->EarlyWSOpenEnabled() && !WebSocket::IsDisabled()) {
            wsMachine = std::make_unique<StateMachine>(*this);
        }

        // Método para notificar que el request de creación de sesión ha sido enviado
        void createSent() override {
            Session::createSent(); // Llama al método base
            wsMachine->createSent();
        }

        // Retorna el estado de conexión alto nivel
        std::string getConnectedHighLevelStatus() const override {
            return isPolling ? "WS_POLLING" : "WS_STREAMING";
        }

        // Retorna el primer estado de conexión
        std::string getFirstConnectedStatus() const override {
            return "SENSE";
        }

        // Indica si debería preguntarse por la longitud del contenido
        bool shouldAskContentLength() const override {
            return false;
        }

        // Envía un heartbeat inverso
        void sendReverseHeartbeat(std::unique_ptr<ReverseHeartbeatRequest> request, std::shared_ptr<RequestTutor> tutor) override {
            // TODO:  Implementación específica para enviar heartbeat inverso
        }

        // Implementación específica para ejecución de sesión de bind
        std::future<void> bindSessionExecution(const std::string& bindCause) {
            // TODO: Debes adaptar esto para iniciar y manejar una sesión de bind
        }

        // Cambia el enlace de control
        void changeControlLink(const std::string& controlLink) override {
            wsMachine->changeControlLink(controlLink);
        }

        // Maneja eventos de error
        void doOnErrorEvent(const std::string& reason, bool closedOnServer, bool unableToOpen, bool startRecovery, long timeLeftMs, bool wsError) override {
            // TODO: Implementación específica para manejar errores
        }

    };

};

} // namespace lightstreamer::client::session

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONWS_HPP
