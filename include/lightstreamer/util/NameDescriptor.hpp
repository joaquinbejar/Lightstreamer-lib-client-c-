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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_NAMEDESCRIPTOR_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_NAMEDESCRIPTOR_HPP
#include <string>
#include <memory>
#include <lightstreamer/util/Descriptor.hpp>

namespace lightstreamer::util {


    class NameDescriptor : public Descriptor {
    private:
        std::string name;

    public:
        NameDescriptor(const std::string& name) : name(name) {}

        int getPos(const std::string& name) const override {
            if (this->subDescriptor) { // Assuming subDescriptor is a smart pointer to a Descriptor
                int fromSub = this->subDescriptor->getPos(name);
                return fromSub > -1 ? fromSub + this->getSize() : -1;
            }
            return -1;
        }

        std::string getName(int pos) const override {
            if (this->subDescriptor) {
                return this->subDescriptor->getName(pos - this->getSize());
            }
            return ""; // Returning empty string instead of nullptr
        }

        std::string ComposedString() const override {
            return this->name;
        }

        std::string Original() const {
            return this->name;
        }

        // Implementing cloning by returning a new instance copied from this one
        std::unique_ptr<NameDescriptor> clone() const {
            return std::make_unique<NameDescriptor>(*this);
        }
    };

    // Make sure subDescriptor is properly defined, either here or in the Descriptor base class.
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_NAMEDESCRIPTOR_HPP
