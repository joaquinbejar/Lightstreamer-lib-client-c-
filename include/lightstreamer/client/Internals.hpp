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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_INTERNALS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_INTERNALS_HPP

#include <lightstreamer/client/ItemUpdate.hpp>
#include <lightstreamer/client/Subscription.hpp>
#include <lightstreamer/util/Descriptor.hpp>


namespace lightstreamer::client {

    /**
     * Utility methods to access non-public methods/fields from a package different from the containing package.
     * The methods/fields must remain non-public because the containing classes are documented in a way that
     * they are presented to the final user, who should not see these internal elements.
     */
    class Internals {
    public:
        static util::Descriptor getItemDescriptor(const Subscription &sub) {
            return sub.itemDescriptor;
        }

        static util::Descriptor getFieldDescriptor(const Subscription &sub) {
            return sub.fieldDescriptor;
        }

        static int getRequestedBufferSize(const Subscription &sub) {
            return sub.requestedBufferSize;
        }

        static double getRequestedMaxFrequency(const Subscription &sub) {
            return sub.requestedMaxFrequency;
        }
    };

} // namespace lightstreamer::client

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_INTERNALS_HPP
