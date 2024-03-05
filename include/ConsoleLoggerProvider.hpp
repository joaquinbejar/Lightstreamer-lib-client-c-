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
    /*
    This LoggerProvider rests on the logging facility provided by the standard module *logging*. The log events are
    forwarded to the logger named *lightstreamer*.

    If you need further customizations, you can leverage the features of module *logging* through,
    for example, *logging.basicConfig*::

    logging.basicConfig(level=logging.DEBUG, format="%(message)s", stream=sys.stdout)

    @param level: the threshold of the loggers created by this provider (see \ref `Level`)
    */
    class ConsoleLoggerProvider {
    private:
        Level m_consoleloglevel;

    public:
        ConsoleLoggerProvider(Level consoleloglevel) : m_consoleloglevel(consoleloglevel) {
        }

        std::shared_ptr<ConsoleLogger> getLogger(const Category &category) {
            return ConsoleLogger::getInstance(m_consoleloglevel, category);
        }
    };
}
#endif //CONSOLELOGGGERPROVIDER_HPP
