/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 13/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_DESCRIPTOR_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_DESCRIPTOR_HPP

#include <string>
#include <memory>
#include <stdexcept>
#include <utility>

namespace lightstreamer::util {

    class Descriptor {
    protected:
        std::shared_ptr<Descriptor> subDescriptor = nullptr;
        int length = 0;

    public:
        virtual ~Descriptor() = default;

        void setSubDescriptor(std::shared_ptr<Descriptor> value) {
            subDescriptor = std::move(value);
        }

        [[nodiscard]] std::shared_ptr<Descriptor> getSubDescriptor() const {
            return subDescriptor;
        }

        [[nodiscard]] int getSize() const {
            return length;
        }

        void setSize(int value) {
            length = value;
        }

        [[nodiscard]] int getFullSize() const {
            if (subDescriptor != nullptr) {
                return getSize() + subDescriptor->getSize();
            }
            return getSize();
        }

        [[maybe_unused]] [[nodiscard]] virtual int getPos(const std::string &name) const {
            throw std::runtime_error("Not implemented");
        }

        [[nodiscard]] virtual std::string getName(int pos) const {
            throw std::runtime_error("Not implemented");
        }

        [[nodiscard]] virtual std::string getComposedString() const {
            throw std::runtime_error("Not implemented");
        }

        [[nodiscard]] virtual std::shared_ptr<Descriptor> clone() const {
            // This implementation assumes Descriptor can be copied via its copy constructor.
            // Derived classes should override clone to handle their specific cloning needs.
            if (!subDescriptor) {
                return std::make_shared<Descriptor>(*this);
            } else {
                auto copy = std::make_shared<Descriptor>(*this);
                copy->subDescriptor = std::static_pointer_cast<Descriptor>(subDescriptor->clone());
                return copy;
            }
            // Note: This simplistic approach may need adjustments based on the actual inheritance structure.
        }
    };

} // namespace lightstreamer::util

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_DESCRIPTOR_HPP
