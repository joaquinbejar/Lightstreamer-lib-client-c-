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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_ALTERNATIVELOADER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_ALTERNATIVELOADER_HPP
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <iostream>

template<typename T>
class AlternativeLoader {
public:
    virtual std::vector<std::string> getDefaultClassNames() const = 0;

protected:
    std::map<std::string, std::function<std::shared_ptr<T>()>> factoryMap;

    std::shared_ptr<T> loadImplementation(const std::string& className) {
        auto it = factoryMap.find(className);
        if (it != factoryMap.end()) {
            return it->second();
        }
        return nullptr;
    }

public:
    void registerFactory(const std::string& className, std::function<std::shared_ptr<T>()> factory) {
        factoryMap[className] = factory;
    }

    std::shared_ptr<T> getAlternative() {
        auto alternatives = getDefaultClassNames();
        for (const auto& className : alternatives) {
            auto impl = loadImplementation(className);
            if (impl != nullptr) {
                return impl;
            }
        }
        return nullptr;
    }
};
#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_ALTERNATIVELOADER_HPP
