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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_VISITOR_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_VISITOR_HPP
namespace lightstreamer::util {

    /**
     * Visitor of collections.
     * @tparam T The type of elements in the collection.
     */
    template<typename T>
    class Visitor {
    public:
        /**
         * Executes the operation for each element of a collection.
         * @param listener The element of the collection to visit.
         */
        virtual void visit(T listener) = 0;

        // Ensure the inclusion of a virtual destructor to allow for proper cleanup of derived classes
        virtual ~Visitor() = default;
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_VISITOR_HPP
