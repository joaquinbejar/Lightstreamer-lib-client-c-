/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 28/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_ASSERTIONS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_ASSERTIONS_HPP
#include <thread>
#include <string>
#include <memory>
#include "Logger.hpp"
#include "LogManager.hpp"

namespace lightstreamer::util {

    /**
     * Assertions class for validating thread contexts and logical conditions.
     */
    class Assertions {
    private:
        static inline std::shared_ptr<Logger> log = LogManager::GetLogger("ASSERT");

    public:
        /**
         * Checks if the current thread is the Session Thread.
         * @return true if the current thread is the Session Thread, false otherwise.
         */
        static bool IsSessionThread() {
            if (!std::this_thread::get_id().name().starts_with("Session Thread")) {
                log->Error("The method must be called by Session Thread. Instead the caller is " + std::this_thread::get_id());
                return false;
            }
            return true;
        }

        /**
         * Checks if the current thread is the Events Thread.
         * @return true if the current thread is the Events Thread, false otherwise.
         */
        static bool IsEventThread() {
            if (!std::this_thread::get_id().name().starts_with("Events Thread")) {
                // log->Error("The method must be called by Event Thread. Instead the caller is " + std::this_thread::get_id());
                return false;
            }
            return true;
        }

        /**
         * Checks if the current thread is the Netty Thread.
         * @return true if the current thread is the Netty Thread, false otherwise.
         */
        static bool IsNettyThread() {
            if (!std::this_thread::get_id().name().starts_with("Netty Thread")) {
                // log->Error("The method must be called by Netty Thread. Instead the caller is " + std::this_thread::get_id());
                return false;
            }
            return true;
        }

        /**
         * Conditional operator.
         * @param a The antecedent.
         * @param b The consequent.
         * @return true if the implication a -> b is true, false otherwise.
         */
        static bool implies(bool a, bool b) {
            return !a || b;
        }

        /**
         * Biconditional operator.
         * @param a The first operand.
         * @param b The second operand.
         * @return true if a if and only if b, false otherwise.
         */
        static bool iff(bool a, bool b) {
            return a == b;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_ASSERTIONS_HPP
