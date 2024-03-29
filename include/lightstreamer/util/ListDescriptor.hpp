/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 29/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_LISTDESCRIPTOR_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_LISTDESCRIPTOR_HPP
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <lightstreamer/util/Descriptor.hpp>

namespace lightstreamer::util {

    class ListDescriptor : public Descriptor {
    private:
        std::vector<std::string> list;
        std::unordered_map<std::string, int> reverseList;

        static std::unordered_map<std::string, int> getReverseList(const std::vector<std::string>& list) {
            std::unordered_map<std::string, int> reverseList;
            for (size_t i = 0; i < list.size(); ++i) {
                reverseList[list[i]] = static_cast<int>(i) + 1;
            }
            return reverseList;
        }

        // Error messages
        static const std::string NO_EMPTY;
        static const std::string NO_SPACE;
        static const std::string NO_NUMBER;

    public:
        ListDescriptor(const std::vector<std::string>& list)
                : list(list), reverseList(getReverseList(list)) {
            Size = static_cast<int>(list.size()); //TODO: Size is not defined in Descriptor
        }

        // Deleted set function for Size to preserve constness
        void setSize(int newSize) = delete;

        std::string ComposedString() const override {
            if (list.empty()) return "";

            std::ostringstream joined;
            auto it = list.begin();
            joined << *it++;
            for (; it != list.end(); ++it) {
                joined << " " << *it;
            }

            return joined.str();
        }

        int getPos(const std::string& name) const override {
            auto it = reverseList.find(name);
            if (it != reverseList.end()) {
                return it->second;
            } else if (subDescriptor) { // Assuming subDescriptor is defined somewhere in Descriptor
                int fromSub = subDescriptor->getPos(name);
                return fromSub > -1 ? fromSub + getSize() : -1;
            }
            return -1;
        }

        std::string getName(int pos) const override {
            if (pos > getSize() ) {
                if (subDescriptor) { // Assuming subDescriptor is defined
                    return subDescriptor->getName(pos - getSize() );
                }
            } else if (pos >= 1) {
                return list[pos - 1];
            }
            return ""; // Returning empty string instead of null
        }

        std::vector<std::string> Original() const {
            return list;
        }

        static void checkItemNames(const std::vector<std::string>& names, const std::string& head) {
            for (const auto& name : names) {
                if (name.empty()) {
                    throw std::invalid_argument(head + NO_EMPTY);
                } else if (name.find(' ') != std::string::npos) {
                    throw std::invalid_argument(head + NO_SPACE);
                } else if (std::all_of(name.begin(), name.end(), ::isdigit)) {
                    throw std::invalid_argument(head + NO_NUMBER);
                }
            }
        }

        static void checkFieldNames(const std::vector<std::string>& names, const std::string& head) {
            for (const auto& name : names) {
                if (name.empty()) {
                    throw std::invalid_argument(head + NO_EMPTY);
                } else if (name.find(' ') != std::string::npos) {
                    throw std::invalid_argument(head + NO_SPACE);
                }
            }
        }

        // Assuming the presence of a copy or clone functionality in the base Descriptor
        ListDescriptor* clone() const {
            auto* clone = new ListDescriptor(*this); // Shallow copy; adjust if deep copy is needed
            return clone;
        }
    };

    const std::string ListDescriptor::NO_EMPTY = " name cannot be empty";
    const std::string ListDescriptor::NO_SPACE = " name cannot contain spaces";
    const std::string ListDescriptor::NO_NUMBER = " name cannot be a number";
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_LISTDESCRIPTOR_HPP
