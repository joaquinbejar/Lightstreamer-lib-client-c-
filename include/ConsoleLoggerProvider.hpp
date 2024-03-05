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
    Date: 2/3/24
 ******************************************************************************/
#ifndef CONSOLELOGGGERPROVIDER_HPP
#define CONSOLELOGGGERPROVIDER_HPP

#include <ConsoleLogLevel.hpp>
#include <Logger.hpp>

using Lightstreamer::Cpp::ConsoleLogLevel::Level;
using namespace Lightstreamer::Cpp::Logger;

namespace Lightstreamer::Cpp::ConsoleLoggerProvider {
    /**
     * @class ConsoleLoggerProvider
    * @brief  This ConsoleLoggerProvider rests on the logging facility provided by the standard module @b logging. The log events are forwarded to the logger named @b lightstreamer.
     *
     * The ConsoleLoggerProvider class provides a way to obtain a console logger.
     */
    class ConsoleLoggerProvider {
    private:
        Level m_consoleloglevel;

    public:
        /**
         * @brief Constructs a ConsoleLoggerProvider object with the specified console log level.
         *
         * @param consoleloglevel The console log level to be set for the logger provider.
         */
        explicit ConsoleLoggerProvider(const Level consoleloglevel) : m_consoleloglevel(consoleloglevel) {
        }

        /**
         * @brief Retrieves a shared pointer to a ConsoleLogger instance.
         *
         * This method retrieves a shared pointer to a ConsoleLogger instance
         * associated with the given category. The ConsoleLogger instance is
         * created using the current console log level and the specified category.
         *
         * @param category The category associated with the ConsoleLogger instance.
         * @return A shared pointer to the ConsoleLogger instance.
         * @note The returned shared pointer should be managed by the caller.
         * @warning Calling this method without setting the console log level
         *          may result in an unexpected behavior.
         */
        [[nodiscard]] std::shared_ptr<ConsoleLogger> getLogger(const Category &category) const {
            return ConsoleLogger::getInstance(m_consoleloglevel, category);
        }
    };
}
#endif //CONSOLELOGGGERPROVIDER_HPP
