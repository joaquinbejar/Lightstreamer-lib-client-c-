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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_HASHMAPHELPER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_HASHMAPHELPER_HPP
#include <map>
#include <set>
#include <utility> // For std::pair

template<typename TKey, typename TValue>
class HashMapHelper {
public:
    // Mimics the SetOfKeyValuePairs method by returning a set of key-value pairs from a map
    static std::set<std::pair<TKey, TValue>> SetOfKeyValuePairs(const std::map<TKey, TValue>& dictionary) {
        std::set<std::pair<TKey, TValue>> entries;
        for (const auto& keyValuePair : dictionary) {
            entries.insert(keyValuePair);
        }
        return entries;
    }

    // Mimics the GetValueOrNull method by returning the value for a given key or a default value if the key does not exist
    static TValue GetValueOrNull(const std::map<TKey, TValue>& dictionary, const TKey& key) {
        auto it = dictionary.find(key);
        if (it != dictionary.end()) {
            return it->second;
        } else {
            return TValue(); // Return default-constructed instance of TValue
        }
    }
};

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_HASHMAPHELPER_HPP
