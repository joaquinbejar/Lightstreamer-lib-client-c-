/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 2/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_ITEMUPDATE_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_ITEMUPDATE_HPP

#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <exception>
#include <algorithm> // for std::sort
#include <stdexcept> // for std::invalid_argument and std::logic_error
#include <lightstreamer/util/Descriptor.hpp>
#include <lightstreamer/util/NameDescriptor.hpp>

namespace lightstreamer::client {

/**
 * Contains all the information related to an update of the field values for an item.
 * It reports all the new values of the fields.
 *
 * COMMAND Subscription:
 * If the involved Subscription is a COMMAND Subscription, then the values for the current
 * update are meant as relative to the same key.
 * Moreover, if the involved Subscription has a two-level behavior enabled, then each update
 * may be associated with either a first-level or a second-level item. In this case, the reported
 * fields are always the union of the first-level and second-level fields and each single update
 * can only change either the first-level or the second-level fields (but for the "command" field,
 * which is first-level and is always set to "UPDATE" upon a second-level update); note
 * that the second-level field values are always null until the first second-level update
 * occurs). When the two-level behavior is enabled, in all methods where a field name has to
 * be supplied, the following convention should be followed:
 *
 * - The field name can always be used, both for the first-level and the second-level fields.
 *   In case of name conflict, the first-level field is meant.
 * - The field position can always be used; however, the field positions for the second-level
 *   fields start at the highest position of the first-level field list + 1. If a field schema had
 *   been specified for either first-level or second-level Subscriptions, then client-side knowledge
 *   of the first-level schema length would be required.
 */
    class ItemUpdate {
    public:
        // Constructor
        ItemUpdate(std::string itemName, int itemPos, bool isSnapshot, std::vector<std::string> updates,
                   std::set<int> changedFields, std::shared_ptr<util::Descriptor> fields) :
                itemName(std::move(itemName)),
                itemPos(itemPos),
                isSnapshot(isSnapshot),
                updates(std::move(updates)),
                changedFields(std::move(changedFields)),
                fields(std::move(fields)) {}

        // Accessors
        /**
         * Read-only property `ItemName` represents the name of the item to which this update pertains.
         * The name will be null if the related Subscription was initialized using an "Item Group".
         */
        const std::string &getItemName() const {
            return itemName;
        }

        /**
         * Read-only property `ItemPos` represents the 1-based the position in the "Item List" or
         * "Item Group" of the item to which this update pertains.
         */
        int getItemPos() const {
            return itemPos;
        }

        /**
         * Returns the current value for the specified field.
         */
        std::string getValue(const std::string &fieldName) const {
            int pos = toPos(fieldName);
            if (pos < 1 || pos > updates.size()) {
                throw std::invalid_argument("the specified field position is out of bounds");
            }
            return updates[pos - 1]; // fieldPos is 1 based, updates is 0 based
        }

        /**
         * Returns the current value for the specified field.
         */
        std::string getValue(int fieldPos) const {
            int pos = toPos(fieldPos);
            if (pos < 1 || pos > updates.size()) {
                throw std::invalid_argument("the specified field position is out of bounds");
            }
            return updates[pos - 1]; // fieldPos is 1 based, updates is 0 based
        }

        /**
         * Inquiry method that asks whether the current update belongs to the item snapshot.
         */
        bool isSnapshot() const {
            return isSnapshot;
        }

        /**
         * Inquiry method that asks whether the value for a field has changed after the reception of the last
         * update from the Server for an item. If the Subscription mode is COMMAND then the change is meant as
         * relative to the same key.
         */
        bool isValueChanged(const std::string &fieldName) const {
            int pos = toPos(fieldName);
            return changedFields.find(pos) != changedFields.end();
        }

        /**
         * Inquiry method that asks whether the value for a field has changed after the reception of the last
         * update from the Server for an item. If the Subscription mode is COMMAND then the change is meant as
         * relative to the same key.
         */
        bool isValueChanged(int fieldPos) const {
            int pos = toPos(fieldPos);
            return changedFields.find(pos) != changedFields.end();
        }

        // Add remaining methods and private helpers as needed.

    private:
        std::string itemName;
        int itemPos;
        bool isSnapshot;
        std::shared_ptr<util::Descriptor> fields;
        std::vector<std::string> updates;
        std::set<int> changedFields;

        // Add helper functions for internal use.

        int toPos(const std::string &fieldName) const {
            // Implementation depends on Descriptor and NameDescriptor definitions
            // Placeholder for the actual logic
            return 0;
        }

        int toPos(int fieldPos) const {
            // Placeholder for the actual logic, assuming 'updates' vector is already filled
            return fieldPos;
        }
    };

} // namespace lightstreamer::client

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_ITEMUPDATE_HPP
