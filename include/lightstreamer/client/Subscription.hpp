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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTION_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTION_HPP

#include <lightstreamer/util/Descriptor.hpp>
#include <lightstreamer/util/NameDescriptor.hpp>
#include <lightstreamer/util/ListDescriptor.hpp>
#include <lightstreamer/client/ItemUpdate.hpp>
#include <lightstreamer/client/Constants.hpp>
#include <lightstreamer/client/SubscriptionListener.hpp>
#include <lightstreamer/client/events/EventDispatcher.hpp>
#include "Logger.hpp" // Assuming existence of a Logger class
#include <lightstreamer/client/session/SessionManager.hpp>
#include <lightstreamer/client/session/SessionThread.hpp>

// License information and other comments have been omitted for brevity
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <set>
#include <exception>
#include <iostream>
#include <cassert>


namespace lightstreamer::client {
    /**
     * @brief Class representing a Subscription to be submitted to a Lightstreamer Server. It contains
     * subscription details and the listeners needed to process the real-time data.
     *
     * After creation, a Subscription object is in the "inactive" state. When a Subscription
     * object is subscribed to a LightstreamerClient object, through the
     * LightstreamerClient::subscribe(std::shared_ptr<Subscription>) method, its state becomes "active".
     * This means that the client activates a subscription to the required items through
     * Lightstreamer Server, and the Subscription object begins to receive real-time events.
     *
     * A Subscription can be configured to use either an Item Group or an Item List to specify the
     * items to be subscribed to and using either a Field Schema or Field List to specify the fields.
     *
     * "Item Group" and "Item List" are defined as follows:
     * - "Item Group": an Item Group is a string identifier representing a list of items.
     *   Such Item Group has to be expanded into a list of items by the getItems method of the
     *   MetadataProvider of the associated Adapter Set. When using an Item Group, items in the
     *   subscription are identified by their 1-based index within the group.
     *   It is possible to configure the Subscription to use an "Item Group" leveraging the
     *   Subscription::setItemGroup(const std::string&) method.
     *
     * - "Item List": an Item List is a vector of strings, each one representing an item.
     *   For the Item List to be correctly interpreted, a LiteralBasedProvider or a MetadataProvider
     *   with a compatible implementation of getItems has to be configured in the associated
     *   Adapter Set.
     *   Note that no item in the list can be empty, can contain spaces, or can be a number.
     *   When using an Item List, items in the subscription are identified by their name or
     *   by their 1-based index within the list.
     *   It is possible to configure the Subscription to use an "Item List" leveraging the
     *   Subscription::setItems(const std::vector<std::string>&) method or by specifying it in the constructor.
     *
     * "Field Schema" and "Field List" are defined as follows:
     * - "Field Schema": a Field Schema is a string identifier representing a list of fields.
     *   Such Field Schema has to be expanded into a list of fields by the getFields method of
     *   the MetadataProvider of the associated Adapter Set. When using a Field Schema, fields
     *   in the subscription are identified by their 1-based index within the schema.
     *   It is possible to configure the Subscription to use a "Field Schema" leveraging the
     *   Subscription::setFieldSchema(const std::string&) method.
     *
     * - "Field List": a Field List is a vector of strings, each one representing a field.
     *   For the Field List to be correctly interpreted, a LiteralBasedProvider or a MetadataProvider
     *   with a compatible implementation of getFields has to be configured in the associated
     *   Adapter Set.
     *   Note that no field in the list can be empty or can contain spaces.
     *   When using a Field List, fields in the subscription are identified by their name or
     *   by their 1-based index within the list.
     *   It is possible to configure the Subscription to use a "Field List" leveraging the
     *   Subscription::setFields(const std::vector<std::string>&) method or by specifying it in the constructor.
     */
    class Subscription {
    private:
        static constexpr char const *NO_ITEMS = "Please specify a valid item or item list";
        static constexpr char const *NO_FIELDS = "Invalid Subscription, please specify a field list or field schema";
        static constexpr char const *IS_ALIVE = "Cannot modify an active Subscription, please unsubscribe before applying any change";
        static constexpr char const *NOT_ALIVE = "Subscription is not active";
        static constexpr char const *INVALID_MODE = "The given value is not a valid subscription mode. Admitted values are MERGE, DISTINCT, RAW, COMMAND";
        static constexpr char const *NO_VALID_FIELDS = "Please specify a valid field list";
        static constexpr char const *NO_GROUP_NOR_LIST = "The  item list/item group of this Subscription was not initiated";
        static constexpr char const *NO_SCHEMA_NOR_LIST = "The field list/field schema of this Subscription was not initiated";
        static constexpr char const *MAX_BUF_EXC = "The given value is not valid for this setting; use null, 'unlimited' or a positive integer instead";
        static constexpr char const *NO_SECOND_LEVEL = "Second level field list is only available on COMMAND Subscriptions";
        static constexpr char const *NO_COMMAND = "This method can only be used on COMMAND subscriptions";
        static constexpr char const *NO_SUB_SCHEMA_NOR_LIST = "The second level of this Subscription was not initiated";
        static constexpr char const *RAW_NO_SNAPSHOT = "Snapshot is not permitted if RAW was specified as mode";
        static constexpr char const *NUMERIC_DISTINCT_ONLY = "Numeric values are only allowed when the subscription mode is DISTINCT";
        static constexpr char const *REQ_SNAP_EXC = "The given value is not valid for this setting; use null, 'yes', 'no' or a positive number instead";
        static constexpr char const *ILLEGAL_FREQ_EXC = "Can't change the frequency from/to 'unfiltered' or to null while the Subscription is active";
        static constexpr char const *MAX_FREQ_EXC = "The given value is not valid for this setting; use null, 'unlimited', 'unfiltered' or a positive number instead";
        static constexpr char const *INVALID_SECOND_LEVEL_KEY = "The received key value is not a valid name for an Item";

        static constexpr char const *SIMPLE = "SIMPLE";
        static constexpr char const *METAPUSH = "METAPUSH";
        static constexpr char const *MULTIMETAPUSH = "MULTIMETAPUSH";

        static constexpr char const *OFF = "OFF";
        static constexpr char const *WAITING = "WAITING";
        static constexpr char const *PAUSED = "PAUSED";
        static constexpr char const *SUBSCRIBING = "SUBSCRIBING";
        static constexpr char const *PUSHING = "PUSHING";

        static constexpr int FREQUENCY_NULL = -2;
        static constexpr int FREQUENCY_UNFILTERED = -1;
        static constexpr int FREQUENCY_UNLIMITED = 0;

        static constexpr int BUFFER_NULL = -1;
        static constexpr int BUFFER_UNLIMITED = 0;

        static constexpr bool CLEAN = true;
        static constexpr bool DONT_CLEAN = false;

        std::shared_ptr<Logger> log = LogManager::getLogger("ACTIONS_LOG");
        std::shared_ptr<Logger> logStats = LogManager::getLogger("STATS_LOG");

        events::EventDispatcher<SubscriptionListener> dispatcher;

        bool isActive = false;

        std::unique_ptr<util::Descriptor> itemDescriptor;
        std::unique_ptr<util::Descriptor> fieldDescriptor;
        int commandCode = -1;
        int keyCode = -1;

        int nextReconfId = 1;

        // Datos
        std::string dataAdapter;
        std::string mode;
        std::string isRequiredSnapshot;
        std::string selector;
        int requestedBufferSize = BUFFER_NULL;

        ConcurrentMatrix<int, int> oldValuesByItem;
        ConcurrentMatrix<std::string, int> oldValuesByKey;

        std::string underDataAdapter;
        std::unique_ptr<util::Descriptor> subFieldDescriptor;
        std::unordered_map<int, std::unordered_map<std::string, std::shared_ptr<Subscription>>> subTables;

        std::string subMode = "MERGE";
        double aggregatedRealMaxFrequency = FREQUENCY_NULL;
        bool subTableFlag = false;

        static ConcurrentMap<std::string, int> subStats;

        std::string behavior;
        double requestedMaxFrequency = FREQUENCY_NULL;
        double localRealMaxFrequency = FREQUENCY_NULL;

        int subscriptionId = -1;

        std::string tablePhaseType = OFF;
        int tablePhase = 0;
        std::shared_ptr<session::SubscriptionManager> manager;
        std::shared_ptr<session::SessionThread> sessionThread;
        std::vector<std::unique_ptr<SnapshotManager>> snapshotByItem;


        std::vector<std::string> items;
        std::vector<std::string> fields;
        mutable std::mutex mtx;
        bool isActive_{false};


    public:
        /**
         * @brief Creates an object to be used to describe a Subscription that is going to be subscribed to
         * through Lightstreamer Server. The object can be supplied to
         * LightstreamerClient::subscribe() and LightstreamerClient::unsubscribe(), in order to bring the Subscription
         * to "active" or back to "inactive" state.
         *
         * Note that all of the methods used to describe the subscription to the server can only be
         * called while the instance is in the "inactive" state; the only exception is
         * Subscription::requestedMaxFrequency().
         *
         * @param subscriptionMode The subscription mode for the items, required by Lightstreamer Server.
         * Permitted values are: MERGE, DISTINCT, RAW, COMMAND.
         * @param items A vector of items to be subscribed to through Lightstreamer server.
         * It is also possible specify the "Item List" or "Item Group" later.
         * @param fields A vector of fields for the items to be subscribed to through Lightstreamer Server.
         */
        Subscription(const std::string &subscriptionMode, const std::vector<std::string> &items,
                     const std::vector<std::string> &fields) {
            init(subscriptionMode, items, fields);
        }

        /**
         * @brief Creates an object to be used to describe a Subscription with a single item.
         *
         * @param subscriptionMode The subscription mode for the items, required by Lightstreamer Server.
         * @param item The item name to be subscribed to through Lightstreamer Server.
         * @param fields A vector of fields for the item to be subscribed to through Lightstreamer Server.
         */
        Subscription(const std::string &subscriptionMode, const std::string &item,
                     const std::vector<std::string> &fields) {
            init(subscriptionMode, std::vector<std::string>{item}, fields);
        }

        /**
         * @brief Creates an object to be used to describe a Subscription without specifying items or fields.
         *
         * @param subscriptionMode The subscription mode for the items, required by Lightstreamer Server.
         */
        Subscription(const std::string &subscriptionMode) {
            init(subscriptionMode, {}, {});
        }

    private:
        void init(const std::string &subscriptionMode, const std::vector<std::string> &items,
                  const std::vector<std::string> &fields) {
            if (subscriptionMode.empty()) {
                throw std::invalid_argument("INVALID_MODE");
            }

            std::string upperSubscriptionMode = subscriptionMode;
            std::transform(upperSubscriptionMode.begin(), upperSubscriptionMode.end(), upperSubscriptionMode.begin(),
                           ::toupper);

            // Assuming Constants::MODES contains the valid modes
            if (std::find(Constants::MODES.begin(), Constants::MODES.end(), upperSubscriptionMode) ==
                Constants::MODES.end()) {
                throw std::invalid_argument("INVALID_MODE");
            }

            this->mode = upperSubscriptionMode;
            this->isRequiredSnapshot = (this->mode == "RAW") ? "" : "yes";
            this->behavior = (this->mode == "COMMAND") ? "METAPUSH" : "SIMPLE";

            if (!items.empty()) {
                if (fields.empty()) {
                    throw std::invalid_argument("NO_VALID_FIELDS");
                }
                this->items = items;
                this->fields = fields;
            } else if (!fields.empty()) {
                throw std::invalid_argument("NO_ITEMS");
            }
        }

    public:
        void addListener(std::shared_ptr<SubscriptionListener> listener) {
            std::lock_guard<std::mutex> guard(mtx);
            dispatcher.addListener(listener);
        }

        void removeListener(std::shared_ptr<SubscriptionListener> listener) {
            std::lock_guard<std::mutex> guard(mtx);
            dispatcher.removeListener(listener);
        }

        std::vector<std::shared_ptr<SubscriptionListener>> getListeners() {
            std::lock_guard<std::mutex> guard(mtx);
            return dispatcher.getListeners();
        }

        /**
         * @brief Checks if the Subscription is currently "active" or not.
         *
         * Most of the Subscription properties cannot be modified if a Subscription is "active".
         * The status of a Subscription is changed to "active" through the
         * LightstreamerClient::subscribe method and back to "inactive" through the
         * LightstreamerClient::unsubscribe one.
         *
         * @return true if the Subscription is "active", false otherwise.
         */
        bool isActive() const {
            std::lock_guard<std::mutex> guard(mtx);
            return isActive_;
        }

        /**
         * @brief Checks if the Subscription is currently subscribed to through the server or not.
         *
         * This flag is switched to true by server sent Subscription events, and
         * back to false in case of client disconnection,
         * LightstreamerClient::unsubscribe calls and server sent unsubscription events.
         *
         * @return true if the Subscription is subscribed to through the server, false otherwise.
         */
        bool isSubscribed() const {
            std::lock_guard<std::mutex> guard(mtx);
            return tablePhaseType == "PUSHING"; // Simplified version; Adjust according to your implementation.
        }

        /**
         * @brief Gets the name of the Data Adapter (within the Adapter Set used by the current session)
         * that supplies all the items for this Subscription.
         *
         * Note: This method can only be called while the Subscription instance is in its
         * "inactive" state.
         *
         * @return The name of the Data Adapter.
         */
        std::string getDataAdapter() const {
            std::lock_guard<std::mutex> guard(mtx);
            return dataAdapter;
        }

        /**
         * @brief Sets the name of the Data Adapter (within the Adapter Set used by the current session)
         * that supplies all the items for this Subscription.
         *
         * Note: This method can only be called while the Subscription instance is in its
         * "inactive" state.
         *
         * @param adapterName The name of the Data Adapter.
         */
        void setDataAdapter(const std::string &adapterName) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck(); // Ensure Subscription is not active; implement this check as needed.
            dataAdapter = adapterName;
        }

        // Other members and methods...

        /**
         * Returns the mode specified in the constructor for this Subscription.
         *
         * This method can be called at any time.
         *
         * @return The subscription mode.
         */
        std::string getMode() const {
            std::lock_guard<std::mutex> guard(mtx);
            return mode;
        }

        /**
         * Returns the "Item List" to be subscribed to through the Lightstreamer Server.
         *
         * Any call to this property will return the items that have been specified
         * previously. Note that if the single-item constructor was used, this method
         * will return a vector of length 1 containing that item.
         *
         * This method can only be called if the Subscription has been initialized with an "Item List".
         *
         * @return A vector of items.
         */
        std::vector<std::string> getItems() const {
            std::lock_guard<std::mutex> guard(mtx);
            if (!itemDescriptor) {
                throw std::invalid_argument("NO_GROUP_NOR_LIST");
            }
            if (dynamic_cast<NameDescriptor *>(itemDescriptor.get())) {
                throw std::invalid_argument(
                        "This Subscription was initiated using an item group, use getItemGroup instead of using getItems");
            }
            return static_cast<ListDescriptor *>(itemDescriptor.get())->getOriginal();
        }

        void setItems(const std::vector<std::string> &newItems) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck();
            // Validation of item names would go here
            itemDescriptor = std::make_unique<ListDescriptor>(newItems);
        }

        /**
         * Returns the "Item Group" to be subscribed to through the Lightstreamer Server.
         *
         * Any call to this property will override any "Item List" or "Item Group" previously specified.
         *
         * This method can only be called if the Subscription has been initialized using an "Item Group".
         *
         * @return The name of the item group.
         */
        std::string getItemGroup() const {
            std::lock_guard<std::mutex> guard(mtx);
            if (!itemDescriptor) {
                throw std::invalid_argument("NO_GROUP_NOR_LIST");
            }
            if (dynamic_cast<ListDescriptor *>(itemDescriptor.get())) {
                throw std::invalid_argument(
                        "This Subscription was initiated using an item list, use getItems instead of using getItemGroup");
            }
            return static_cast<NameDescriptor *>(itemDescriptor.get())->getOriginal();
        }

        void setItemGroup(const std::string &newItemGroup) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck();
            itemDescriptor = std::make_unique<NameDescriptor>(newItemGroup);
        }

        /**
         * @brief Get the Field List to be subscribed to through the Lightstreamer Server.
         *
         * Any call to this method will override any "Field List" or "Field Schema" previously specified.
         * This method can be set only while the Subscription instance is in its "inactive" state.
         *
         * @return A vector containing the names of the fields.
         */
        std::vector<std::string> getFields() const {
            std::lock_guard<std::mutex> guard(mtx);
            if (!fieldDescriptor) {
                throw std::runtime_error("NO_SCHEMA_NOR_LIST");
            }
            auto listDescriptor = dynamic_cast<ListDescriptor *>(fieldDescriptor.get());
            if (!listDescriptor) {
                throw std::runtime_error(
                        "This Subscription was initiated using a field schema, use getFieldSchema instead of using getFields");
            }
            return listDescriptor->getOriginal();
        }

        void setFields(const std::vector<std::string> &newFields) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck();
            // Here you would validate the field names
            fieldDescriptor = std::make_unique<ListDescriptor>(newFields);
        }

        /**
         * @brief Get the Field Schema to be subscribed to through the Lightstreamer Server.
         *
         * Any call to this method will override any "Field List" or "Field Schema" previously specified.
         * This method can be set only while the Subscription instance is in its "inactive" state.
         *
         * @return A string representing the name of the field schema.
         */
        std::string getFieldSchema() const {
            std::lock_guard<std::mutex> guard(mtx);
            if (!fieldDescriptor) {
                throw std::runtime_error("NO_SCHEMA_NOR_LIST");
            }
            auto nameDescriptor = dynamic_cast<NameDescriptor *>(fieldDescriptor.get());
            if (!nameDescriptor) {
                throw std::runtime_error(
                        "This Subscription was initiated using a field list, use getFields instead of using getFieldSchema");
            }
            return nameDescriptor->getOriginal();
        }

        void setFieldSchema(const std::string &newFieldSchema) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck();
            fieldDescriptor = std::make_unique<NameDescriptor>(newFieldSchema);
        }

    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTION_HPP
