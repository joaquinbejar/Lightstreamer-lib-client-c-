/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 31/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_VOIDTUTOR_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_VOIDTUTOR_HPP
#include <memory>
#include <lightstreamer/client/requests/RequestTutor.hpp>

namespace lightstreamer::client::requests {

    /**
     * Represents a VoidTutor which always allows sending and doesn't require retransmissions.
     */
    class VoidTutor : public RequestTutor {
    public:
        /**
         * Constructs a VoidTutor with a session thread and connection options.
         *
         * @param thread The session thread.
         * @param connectionOptions The connection options.
         */
        VoidTutor(std::shared_ptr<SessionThread> thread, std::shared_ptr<InternalConnectionOptions> connectionOptions)
                : RequestTutor(thread, connectionOptions) {}

        /**
         * Determines if the request should be sent. Always returns true for VoidTutor.
         *
         * @return Always true.
         */
        bool shouldBeSent() override {
            return true;
        }

        /**
         * Verifies if the operation was successful. Always returns true for VoidTutor.
         *
         * @return Always true.
         */
        bool verifySuccess() override {
            return true;
        }

        /**
         * Performs recovery actions. No action needed for VoidTutor.
         */
        void doRecovery() override {
        }

        /**
         * Notifies about an abort request. No action needed for VoidTutor.
         */
        void notifyAbort() override {
        }

        /**
         * Indicates if the timeout is fixed. Always returns false for VoidTutor.
         *
         * @return Always false.
         */
        bool TimeoutFixed() override {
            return false;
        }

        /**
         * Returns the fixed timeout value. Always returns 0 for VoidTutor.
         *
         * @return Always 0.
         */
        long FixedTimeout() override {
            return 0;
        }

        /**
         * Starts the timeout procedure. Doesn't schedule a task on session thread
         * since the VoidTutor doesn't need retransmissions.
         */
        void startTimeout() override {
            // doesn't schedule a task on session thread since the void tutor doesn't need retransmissions
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_VOIDTUTOR_HPP
