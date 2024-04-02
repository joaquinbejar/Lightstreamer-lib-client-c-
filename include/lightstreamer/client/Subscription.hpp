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
            if (dynamic_cast<util::NameDescriptor *>(itemDescriptor.get())) {
                throw std::invalid_argument(
                        "This Subscription was initiated using an item group, use getItemGroup instead of using getItems");
            }
            return static_cast<util::ListDescriptor *>(itemDescriptor.get())->getOriginal();
        }

        void setItems(const std::vector<std::string> &newItems) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck();
            // Validation of item names would go here
            itemDescriptor = std::make_unique<util::ListDescriptor>(newItems);
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
            if (dynamic_cast<util::ListDescriptor *>(itemDescriptor.get())) {
                throw std::invalid_argument(
                        "This Subscription was initiated using an item list, use getItems instead of using getItemGroup");
            }
            return static_cast<util::NameDescriptor *>(itemDescriptor.get())->getOriginal();
        }

        void setItemGroup(const std::string &newItemGroup) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck();
            itemDescriptor = std::make_unique<util::NameDescriptor>(newItemGroup);
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
            auto listDescriptor = dynamic_cast<util::ListDescriptor *>(fieldDescriptor.get());
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
            fieldDescriptor = std::make_unique<util::ListDescriptor>(newFields);
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
            auto nameDescriptor = dynamic_cast<util::NameDescriptor *>(fieldDescriptor.get());
            if (!nameDescriptor) {
                throw std::runtime_error(
                        "This Subscription was initiated using a field list, use getFields instead of using getFieldSchema");
            }
            return nameDescriptor->getOriginal();
        }

        void setFieldSchema(const std::string &newFieldSchema) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck();
            fieldDescriptor = std::make_unique<util::NameDescriptor>(newFieldSchema);
        }

        /**
         * Represents the length to be requested to the Lightstreamer Server for the internal queuing buffers
         * for the items in the Subscription. This can only be used when the subscription mode is MERGE or DISTINCT
         * and unfiltered dispatching has not been requested. The server may set an upper limit on the size of its
         * internal buffers.
         *
         * This property can only be set while the Subscription instance is in its "inactive" state.
         *
         * @return The length of the internal queuing buffers to be used in the server, or "unlimited" if no limit is requested.
         */
        std::string getRequestedBufferSize() const {
            std::lock_guard<std::mutex> guard(mtx);
            if (requestedBufferSize == BUFFER_NULL) {
                return "null";
            } else if (requestedBufferSize == BUFFER_UNLIMITED) {
                return "unlimited";
            } else {
                return std::to_string(requestedBufferSize);
            }
        }

        void setRequestedBufferSize(const std::string &value) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck();

            if (value == "null") {
                requestedBufferSize = BUFFER_NULL;
            } else if (value == "unlimited") {
                requestedBufferSize = BUFFER_UNLIMITED;
            } else {
                try {
                    int tmp = std::stoi(value);
                    if (tmp < 0) {
                        throw std::invalid_argument(
                                "The given value is not valid for this setting; use 'null', 'unlimited', or a positive integer instead.");
                    }
                    requestedBufferSize = tmp;
                } catch (const std::invalid_argument &e) {
                    throw std::invalid_argument(
                            "The given value is not valid for this setting; use 'null', 'unlimited', or a positive integer instead.");
                }
            }
        }

        /**
          * Enables/disables snapshot delivery request for the items in the Subscription.
          * The snapshot can be requested only if the Subscription mode is MERGE, DISTINCT, or COMMAND.
          *
          * @return A string indicating the snapshot request status ("yes", "no", or a numeric value for DISTINCT mode).
          */
        std::string getRequestedSnapshot() const {
            std::lock_guard<std::mutex> guard(mtx);
            return isRequiredSnapshot;
        }

        void setRequestedSnapshot(const std::string &value) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck();

            std::string lowerValue = value;
            std::transform(value.begin(), value.end(), lowerValue.begin(),
                           [](unsigned char c) { return std::tolower(c); });

            if (lowerValue != "no" && mode == "RAW") {
                throw std::invalid_argument("Snapshot is not permitted if RAW was specified as mode");
            } else if (lowerValue == "yes" || lowerValue == "no" || (mode == "DISTINCT" && isNumber(lowerValue))) {
                isRequiredSnapshot = lowerValue;
            } else {
                throw std::invalid_argument(
                        "Invalid value for RequestedSnapshot. Use 'yes', 'no', or a positive number.");
            }
        }

        /**
         * Gets the maximum update frequency that has been requested from the Lightstreamer Server for all items in this Subscription.
         * Can only be used when the Subscription mode is MERGE, DISTINCT, or COMMAND. In COMMAND mode, it limits the update frequency
         * for each key in the subscription.
         *
         * @return A string representing the requested maximum update frequency ("unfiltered", "unlimited", or a numerical rate).
         */
        std::string getRequestedMaxFrequency() const {
            std::lock_guard<std::mutex> guard(mtx);
            if (requestedMaxFrequency == FREQUENCY_UNFILTERED) {
                return "unfiltered";
            } else if (requestedMaxFrequency == FREQUENCY_NULL) {
                return "null"; // In C++, we don't use null for std::string, could use std::optional to represent null.
            } else if (requestedMaxFrequency == FREQUENCY_UNLIMITED) {
                return "unlimited";
            } else {
                return std::to_string(requestedMaxFrequency);
            }
        }

        /**
         * Sets the maximum update frequency to be requested from the Lightstreamer Server for all items in this Subscription.
         * Can only be called when the Subscription is inactive. Attempts to set it while the Subscription is active will throw an exception.
         *
         * @param value The maximum update frequency ("unfiltered", "unlimited", a numerical rate, or "null" to use the server default).
         * @throws std::invalid_argument If the Subscription is currently active.
         */
        void setRequestedMaxFrequency(const std::string &value) {
            std::lock_guard<std::mutex> guard(mtx);
            notAliveCheck();

            double prevValue = requestedMaxFrequency;
            if (value == "null") {
                requestedMaxFrequency = FREQUENCY_NULL;
            } else if (value == "unfiltered") {
                requestedMaxFrequency = FREQUENCY_UNFILTERED;
            } else if (value == "unlimited") {
                requestedMaxFrequency = FREQUENCY_UNLIMITED;
            } else {
                requestedMaxFrequency = std::stod(value); // Exception handling is needed here.
            }

            // Logic to update the frequency on the server, if necessary, would go here.
            if (isActive && prevValue != requestedMaxFrequency) {
                // TODO: Send a request to the server to change the frequency.
                // The actual implementation depends on the server interaction logic.
            }
        }

        // And assuming Matrix is a class that contains Subscriptions and can iterate over them:
        template<typename T1, typename T2, typename T3>
        class Matrix {
        public:
            // An example of what a method might look like that accepts a function to operate on each element
            void forEachElement(const std::function<bool(T3 &element, T1 item, T2 key)> &func) {
                // Iterate over elements and apply the function.
                // This is a simplified example. The actual implementation would depend on how the elements are stored.
                for (auto &[key, element]: elements) {
                    if (!func(element, 0 /* item value */, key)) { // 0 is a placeholder for 'item' value
                        break;
                    }
                }
            }

        private:
            std::unordered_map<T2, T3> elements; // Simplification of how elements might be stored.
        };

        // Now, assuming you want to replicate the functionality of ElementCallbackAnonymousInnerClassS using C++:
        void
        applyFrequencyToSubscriptions(Matrix<int, std::string, Subscription> &matrix, const std::string &frequency) {
            matrix.forEachElement([&frequency](Subscription &secondLevelSubscription, int item, std::string key) {
                secondLevelSubscription.setRequestedMaxFrequency(frequency);
                return false; // Continue iterating. Change to true if you want to stop the iteration early.
            });
        }


        /**
         * Gets the selector name for all items in this Subscription. The selector acts as a filter on the updates received,
         * executed on the server and implemented by the Metadata Adapter.
         *
         * @return The name of the selector or an empty string if no selector is set.
         */
        std::string getSelector() const {
            std::lock_guard<std::mutex> lock(mtx);
            return selector;
        }

        /**
         * Sets the selector name for all items in this Subscription. Can only be called when the Subscription instance is inactive.
         *
         * @param value The name of the selector or an empty string to unset the selector.
         * @throws std::logic_error If the Subscription is currently active.
         */
        void setSelector(const std::string &value) {
            std::lock_guard<std::mutex> lock(mtx);
            notAliveCheck(); // Assume this method throws if the subscription is active.
            selector = value;
            // Logging logic here...
        }

        /**
         * @brief Gets the position of the "command" field in a COMMAND Subscription.
         *
         * This method can only be used if the Subscription mode is COMMAND and the Subscription was initialized using a "Field Schema".
         * The value is the 1-based position of the "command" field within the "Field Schema".
         *
         * @throw std::logic_error If the subscription is not in the correct state or the command field's position is unknown.
         * @return int The 1-based position of the "command" field.
         */
        int getCommandPosition() const {
            std::lock_guard<std::mutex> lock(mtx); // Ensure thread-safe access

            // Assuming commandCheck() verifies that we are in COMMAND mode with a Field Schema and throws if not
            commandCheck();

            if (/* fieldDescriptor is ListDescriptor */) {
                throw std::logic_error(
                        "This Subscription was initiated using a field list, command field is always 'command'");
            }

            if (commandCode == -1) {
                throw std::logic_error("The position of the command field is currently unknown");
            }

            return commandCode;
        }

        /**
         * @brief Gets the position of the "key" field in a COMMAND Subscription.
         *
         * This method can only be used if the Subscription mode is COMMAND and the Subscription was initialized using a "Field Schema".
         * The value is the 1-based position of the "key" field within the "Field Schema".
         *
         * @throw std::logic_error If the subscription is not in the correct state or the key field's position is unknown.
         * @return int The 1-based position of the "key" field.
         */
        int getKeyPosition() const {
            std::lock_guard<std::mutex> lock(mtx); // Ensure thread-safe access

            // Assuming commandCheck() verifies that we are in COMMAND mode with a Field Schema and throws if not
            commandCheck();

            if (/* fieldDescriptor is ListDescriptor */) {
                throw std::logic_error("This Subscription was initiated using a field list, key field is always 'key'");
            }

            if (keyCode == -1) {
                throw std::logic_error("The position of the key field is currently unknown");
            }

            return keyCode;
        }

        /**
         * Retrieves the name of the second-level Data Adapter (within the Adapter Set used by the current session)
         * that supplies all the second-level items. All the possible second-level items should be supplied in "MERGE"
         * mode with snapshot available. The Data Adapter name is configured on the server side through the "name"
         * attribute of the "data_provider" element, in the "adapters.xml" file that defines the Adapter Set (a missing
         * attribute configures the "DEFAULT" name). A null value is equivalent to the "DEFAULT" name. This property
         * can only be called while the Subscription instance is in its "inactive" state. The default value is the
         * default Data Adapter for the Adapter Set, configured as "DEFAULT" on the Server.
         *
         * @return The name of the second-level Data Adapter or "DEFAULT" if not set.
         */
        std::string getCommandSecondLevelDataAdapter() {
            std::lock_guard<std::mutex> lock(mtx);
            return underDataAdapter;
        }

        /**
         * Sets the name of the second-level Data Adapter (within the Adapter Set used by the current session)
         * that supplies all the second-level items. This method can only be called while the Subscription instance
         * is in its "inactive" state.
         *
         * @param value The name of the Data Adapter.
         */
        void setCommandSecondLevelDataAdapter(const std::string &value) {
            std::lock_guard<std::mutex> lock(mtx);
            checkNotAlive(); // Assume existence of a method to check if the subscription is not active
            underDataAdapter = value.empty() ? "DEFAULT" : value;
        }

        /**
         * Retrieves the "Field List" to be subscribed to through Lightstreamer Server for the second-level items.
         * It can only be used on COMMAND Subscriptions. Any call to this method will override any "Field List" or
         * "Field Schema" previously specified for the second-level. Calling this method enables the two-level
         * behavior. Specifying null as parameter will disable the two-level behavior. This property can only be called
         * while the Subscription instance is in its "inactive" state.
         *
         * @return A vector of strings representing the second-level fields.
         */
        std::vector<std::string> getCommandSecondLevelFields() {
            std::lock_guard<std::mutex> lock(mtx);
            if (!subFieldDescriptor) {
                throw std::logic_error("Second-level fields are not set.");
            }
            // Assuming ListDescriptor stores fields as a vector of strings
            return dynamic_cast<util::ListDescriptor *>(subFieldDescriptor.get())->getFields();
        }

        /**
         * Sets the "Field List" for second-level items. This method enables the two-level behavior of a COMMAND
         * Subscription. It can only be called when the Subscription instance is in its "inactive" state.
         *
         * @param fields The list of fields for second-level items.
         */
        void setCommandSecondLevelFields(const std::vector<std::string> &fields) {
            std::lock_guard<std::mutex> lock(mtx);
            checkNotAlive(); // Check if subscription is inactive
            checkCommandMode(); // Assume existence of a method to ensure this is a COMMAND subscription

            // Convert vector to ListDescriptor and store
            subFieldDescriptor = std::make_unique<util::ListDescriptor>(fields);
        }

        /**
         * Retrieves the "Field Schema" to be subscribed to through Lightstreamer Server for the second-level items.
         * It can only be used on COMMAND Subscriptions. Calling this method enables the two-level behavior.
         * Specifying null as parameter will disable the two-level behavior. This property can only be called while the
         * Subscription instance is in its "inactive" state.
         *
         * @return The name of the second-level field schema.
         */
        std::string getCommandSecondLevelFieldSchema() {
            std::lock_guard<std::mutex> lock(mtx);
            if (!subFieldDescriptor) {
                throw std::logic_error("Second-level field schema is not set.");
            }
            // Assuming NameDescriptor stores a single string for the schema
            return dynamic_cast<util::NameDescriptor *>(subFieldDescriptor.get())->getName();
        }

        /**
         * Sets the "Field Schema" for second-level items. This method enables the two-level behavior of a COMMAND
         * Subscription. It can only be called when the Subscription instance is in its "inactive" state.
         *
         * @param schema The name of the field schema for second-level items.
         */
        void setCommandSecondLevelFieldSchema(const std::string &schema) {
            std::lock_guard<std::mutex> lock(mtx);
            checkNotAlive(); // Check if subscription is inactive
            checkCommandMode(); // Ensure this is a COMMAND subscription

            // Convert schema to NameDescriptor and store
            subFieldDescriptor = std::make_unique<util::NameDescriptor>(schema);
        }

        /**
         * Retrieves the latest value received for the specified item/field pair.
         * It's recommended to consume real-time data through a properly implemented SubscriptionListener instead of directly probing this method.
         * In the case of COMMAND subscriptions, the returned value by this method may be misleading, as all received keys, being part of the same item, will overwrite each other.
         * This method can be called at any time; if called to retrieve a value not yet received, it will return an empty optional.
         *
         * @param itemPos The 1-based position of an item within the "Item Group" or "Item List".
         * @param fieldName The field within the "Field List".
         * @return The latest value for the specified field of the specified item, or an empty optional if no value has been received yet.
         */
        std::optional<std::string> getValue(int itemPos, const std::string &fieldName) {
            std::lock_guard<std::mutex> guard(mutex);
            verifyItemPos(itemPos);
            int fieldPos = toFieldPos(fieldName);
            auto &itemMap = oldValuesByItem[itemPos];
            auto it = itemMap.find(fieldPos);
            if (it != itemMap.end()) {
                return it->second;
            }
            return {};
        }

        /**
         * Retrieves the latest value received for a specific item/key/field combination in COMMAND mode subscriptions.
         * It supports two-level behavior, hence the field can be either first-level or second-level.
         * Real-time data consumption through a SubscriptionListener is suggested over direct method calls.
         * Internal data is cleared upon unsubscription.
         *
         * @param itemPos The 1-based position of an item within the "Item Group" or "Item List".
         * @param keyValue The key value received on the COMMAND subscription.
         * @param fieldPos The 1-based position of a field within the "Field Schema" or "Field List".
         * @return The latest value for the specified field of the specified key within the item, or an empty optional if the key hasn't been added (or was deleted).
         */
        std::optional<std::string> getCommandValue(int itemPos, const std::string &keyValue, int fieldPos) {
            std::lock_guard<std::mutex> guard(mutex);
            commandCheck();
            verifyItemPos(itemPos);
            verifyFieldPos(fieldPos, true);

            std::string mapKey = std::to_string(itemPos) + " " + keyValue;
            auto &keyMap = oldValuesByKey[mapKey];
            auto it = keyMap.find(fieldPos);
            if (it != keyMap.end()) {
                return it->second;
            }
            return {};
        }

        void notAliveCheck() {
            if (isActive) {
                throw std::invalid_argument(
                        "Cannot modify an active Subscription, please unsubscribe before applying any change");
            }
        }

        void isAliveCheck() {
            if (!isActive) {
                throw std::invalid_argument("Subscription is not active");
            }
        }

        void setActive() {
            notAliveCheck();
            // Assuming checks for itemDescriptor and fieldDescriptor.
            isActive = true;
        }

        void setInactive() {
            isAliveCheck();
            isActive = false;
        }

        int getSubscriptionId() const {
            return subscriptionId;
        }

        bool is(const std::string &what) const {
            return tablePhaseType == what;
        }

        bool isNot(const std::string &what) const {
            return !is(what);
        }

        void setPhase(const std::string &what) {
            tablePhaseType = what;
            ++tablePhase;
        }

        int getPhase() const {
            return tablePhase;
        }

        bool checkPhase(int phase) const {
            return phase == tablePhase;
        }

        void onAdd(int subId /*, SubscriptionManager manager, SessionThread sessionThread */) {
            if (isNot("OFF")) {
                logError("Add event already executed");
            }
            subscriptionId = subId;
            // Set manager and sessionThread here.
            setPhase("WAITING");
            logDebug("Subscription " + std::to_string(subId) + " ready to be sent to server");
        }

        void onStart() {
            if (isNot("PAUSED")) {
                logError("Unexpected start while not paused");
            }
            setPhase("WAITING");
            logDebug("Subscription " + std::to_string(subscriptionId) + " ready to be sent to server");
        }

        void onRemove() {
            bool wasSubscribed = is("PUSHING");
            logDebug("set OFF sub on Remove.");
            setPhase("OFF");
            // Assume dispatcher.dispatchEvent is handled elsewhere.
            // if (wasSubscribed) { ... }
            // if (behavior == "MULTIMETAPUSH") { ... }
            // cleanData();
            logDebug("Subscription " + std::to_string(subscriptionId) + " is now off");
        }

        void onPause() {
            if (is("OFF")) {
                log("Unexpected pause");
            }

            log("Set PAUSED sub on Pause.");

            bool wasSubscribed = is("PUSHING");
            setPhase("PAUSED");

            // Assume dispatcher.dispatchEvent and other necessary operations are handled
            log("Subscription " + std::to_string(subscriptionId) + " is now on hold");
        }

        void onSubscriptionSent() {
            if (is("SUBSCRIBING")) {
                // first subscribe failed, try again
                return;
            } else if (isNot("WAITING")) {
                log("Was not expecting the subscription request");
            }
            setPhase("SUBSCRIBING");
            log("Subscription " + std::to_string(subscriptionId) + " sent to server");
        }

        void unsupportedCommandWithFieldSchema() {
            setPhase("PAUSED");
            // Dispatch event indicating the error
            // manager.remove(this); // Remove this subscription
        }

        void onSubscriptionAck() {
            setPhase("PUSHING");
        }

        void onSubscribed(int commandPos, int keyPos, int items, int fields) {
            setPhase("PUSHING");

            // Perform necessary setup for the subscription based on the arguments and current state
            // Dispatch the subscription event
            log("Subscription " + std::to_string(subscriptionId) + " is now pushing");
        }

        void onSubscriptionError(int code, const std::string &message) {
            if (isNot("SUBSCRIBING")) {
                log("Was not expecting the error event");
            }

            setPhase("PAUSED");
            // Dispatch the error event with provided code and message
        }


        bool isOff() const {
            return is("OFF");
        }

        bool isWaiting() const {
            return is("WAITING");
        }

        bool isPaused() const {
            return is("PAUSED");
        }

        bool isSubscribing() const {
            return is("SUBSCRIBING");
        }

        bool checkStatusForUpdate() {
            if (!isActive) {
                // Not active, ignore update
                return false;
            } else if (!is("PUSHING")) {
                // Active but not pushing, ignore update
                return false;
            }
            // Active and pushing, process update
            return true;
        }

        // Assuming `SubscribeRequest` and `ChangeSubscriptionRequest` are classes you've implemented
        SubscribeRequest generateSubscribeRequest() {
            return SubscribeRequest(subscriptionId, mode, /* Other parameters as required */);
        }

        ChangeSubscriptionRequest generateFrequencyRequest() {
            return ChangeSubscriptionRequest(subscriptionId, requestedMaxFrequency, ++nextReconfId);
        }

        ChangeSubscriptionRequest generateFrequencyRequest(int reconfId) {
            return ChangeSubscriptionRequest(subscriptionId, requestedMaxFrequency, reconfId);
        }

        void prepareSecondLevel() {
            if (/* condition to check if `subFieldDescriptor` is null */) {
                behavior = "METAPUSH"; // Disable second level
            } else {
                behavior = "MULTIMETAPUSH"; // Enable second level
            }
        }

        // Method to handle the end of a snapshot for a specific item
        void endOfSnapshot(int item) {
            if (!checkStatusForUpdate()) {
                return;
            }

            std::string name = itemDescriptor->getName(item);
            snapshotByItem[item].endOfSnapshot();
            dispatcher.dispatchEvent(SubscriptionListenerEndOfSnapshotEvent(name, item));
        }

        // Method to clear the snapshot for a specific item
        void clearSnapshot(int item) {
            if (!checkStatusForUpdate()) {
                return;
            }

            std::string name = itemDescriptor->getName(item);
            if (behavior == "METAPUSH") {
                oldValuesByKey.clear();
            } else if (behavior == "MULTIMETAPUSH") {
                oldValuesByKey.clear();
                // Additional second-level handling if required
            }

            dispatcher.dispatchEvent(SubscriptionListenerClearSnapshotEvent(name, item));
        }

        // Method to notify about lost updates for a specific item
        void lostUpdates(int item, int lostUpdates) {
            if (!checkStatusForUpdate()) {
                return;
            }
            std::string name = itemDescriptor->getName(item);
            dispatcher.dispatchEvent(SubscriptionListenerItemLostUpdatesEvent(name, item, lostUpdates));
        }

        // Method to configure the subscription based on server-sent frequency settings
        void configure(const std::string& frequency) {
            if (!checkStatusForUpdate()) {
                return;
            }
            try {
                localRealMaxFrequency = std::stod(frequency); // Converts string to double, may throw std::invalid_argument or std::out_of_range
            } catch (...) {
                // Handle error - Invalid frequency received, log this condition and ignore the frequency update
                localRealMaxFrequency = -1; // Assuming -1 is used to indicate no specific frequency
            }

            // Handle behavior-specific configuration updates
            dispatcher.dispatchEvent(SubscriptionListenerConfigurationEvent(frequency));
        }
        void onLostUpdates(const std::string& relKey, int lostUpdates) {
            if (!checkStatusForUpdate()) {
                return;
            }
            dispatcher.dispatchEvent(SubscriptionListenerCommandSecondLevelItemLostUpdatesEvent(lostUpdates, relKey));
        }

        void onServerError(int code, const std::string& message, const std::string& relKey) {
            if (!checkStatusForUpdate()) {
                return;
            }
            dispatcher.dispatchEvent(SubscriptionListenerCommandSecondLevelSubscriptionErrorEvent(code, message, relKey));
        }

        void update(const std::vector<std::string>& args, int item, bool fromMultison) {
            // Logging and initial checks
            if (!checkStatusForUpdate()) {
                return;
            }

            if (snapshotByItem.count(item) && snapshotByItem[item].update()) {
                // Handle snapshot updates
            }

            std::set<int> changedFields = prepareChangedSet(args);
            std::string key = std::to_string(item);

            if (behavior != "SIMPLE") {
                try {
                    key = organizeMPUpdate(args, item, fromMultison, changedFields);
                } catch (const std::exception& e) {
                    // Handle exception
                }
            }

            // Additional handling for MULTIMETAPUSH behavior not shown for brevity
            // Dispatch update event to listeners
        }

        void cleanData() {
            oldValuesByItem.clear();
            oldValuesByKey.clear();
            snapshotByItem.clear();
            fieldDescriptor.setSize(0);
            itemDescriptor.setSize(0);

            if (behavior == "MULTIMETAPUSH") {
                // Additional cleanup for multi-metapush behavior
            }

            // Logging cleanup completion
        }
        std::set<int> prepareChangedSet(const std::vector<std::string>& args) {
            std::set<int> changedFields;
            for (size_t i = 0; i < args.size(); ++i) {
                if (args[i] != ProtocolConstants::UNCHANGED) {
                    changedFields.insert(static_cast<int>(i + 1));
                }
            }
            return changedFields;
        }

        template <typename K>
        void updateStructure(std::map<K, std::map<int, std::string>>& struct_, const K& key, std::vector<std::string>& args, const std::set<int>& changedFields) {
            std::lock_guard<std::mutex> lock(mtx); // Ensure thread safety
            for (size_t i = 0; i < args.size(); ++i) {
                int fieldPos = static_cast<int>(i + 1);
                std::string value = args[i];

                if (value != ProtocolConstants::UNCHANGED) {
                    struct_[key][fieldPos] = value;
                } else {
                    args[i] = struct_[key][fieldPos];
                }
            }
        }

        std::string organizeMPUpdate(std::vector<std::string>& args, int item, bool fromMultison, std::set<int>& changedFields) {
            std::lock_guard<std::mutex> lock(mtx); // Ensure thread safety
            std::string extendedKey;

            int numFields = static_cast<int>(args.size());
            if (commandCode > numFields || keyCode > numFields) {
                // Log error: key and/or command position not correctly configured
                return "";
            }

            std::string currentKey = args[keyCode - 1];
            if (currentKey == ProtocolConstants::UNCHANGED) {
                extendedKey = std::to_string(item) + " " + oldValuesByItem[item][keyCode];
            } else {
                extendedKey = std::to_string(item) + " " + currentKey;
            }

            // Additional logic not shown for brevity
            return extendedKey;
        }
        /**
         * Handle subscription/unsubscription of second level subscriptions.
         * @param item The item index.
         * @param args The arguments list containing subscription details.
         */
        void handleMultiTableSubscriptions(int item, std::list<std::string> args) {
            std::string key = args[this->keyCode - 1];
            if (key == ProtocolConstants::UNCHANGED) {
                key = this->oldValuesByItem.get(item, this->keyCode);
            }

            std::string itemCommand = args[this->commandCode - 1];
            bool subTableExists = this->hasSubTable(item, key);
            if (Constants::DELETE == itemCommand) {
                if (subTableExists) {
                    this->removeSubTable(item, key, CLEAN);
                    this->onLocalFrequencyChanged();
                }
            } else if (!subTableExists) {
                this->addSubTable(item, key);
                // onLocalFrequencyChanged(); // Useless, therefore commented out
            }
        }

        /**
         * Called when the local frequency changes.
         * Asserts specific conditions and updates frequencies accordingly.
         */
        void onLocalFrequencyChanged() {
            assert(behavior == MULTIMETAPUSH);
            assert(!SubTable);
            double prevRealMaxFrequency = aggregatedRealMaxFrequency;

            aggregatedRealMaxFrequency = localRealMaxFrequency;
            this->subTables.forEachElement(std::make_unique<ElementCallbackAnonymousInnerClass>(this));

            if (aggregatedRealMaxFrequency != prevRealMaxFrequency) {
                std::string frequency;
                if (aggregatedRealMaxFrequency == FREQUENCY_UNLIMITED) {
                    frequency = "unlimited";
                } else if (aggregatedRealMaxFrequency == FREQUENCY_NULL) {
                    frequency = nullptr;
                } else {
                    frequency = std::to_string(aggregatedRealMaxFrequency);
                }
                this->dispatcher.dispatchEvent(std::make_unique<SubscriptionListenerConfigurationEvent>(frequency));
            }
        }

        class ElementCallbackAnonymousInnerClass : public Matrix<int, std::string, std::shared_ptr<Subscription>>::ElementCallback {
        private:
            std::shared_ptr<Subscription> outerInstance;

        public:
            ElementCallbackAnonymousInnerClass(std::shared_ptr<Subscription> outerInstance) : outerInstance(outerInstance) {}

            bool onElement(std::shared_ptr<Subscription> value, int item, std::string key) {
                if (isHigherFrequency(value->localRealMaxFrequency, outerInstance->aggregatedRealMaxFrequency)) {
                    outerInstance->aggregatedRealMaxFrequency = value->localRealMaxFrequency;
                }
                return false;
            }

        private:
            bool isHigherFrequency(double fNew, double fOld) {
                if (fOld == FREQUENCY_UNLIMITED || fNew == FREQUENCY_NULL) {
                    return false;
                } else if (fNew == FREQUENCY_UNLIMITED || fOld == FREQUENCY_NULL) {
                    return true;
                } else {
                    return fNew > fOld;
                }
            }
        };

        /**
         * Adds a sub-table.
         * @param item The item index.
         * @param key The key for the subscription.
         */
        void addSubTable(int item, std::string key) {
            auto secondLevelSubscription = std::make_shared<Subscription>(this->subMode);
            secondLevelSubscription->makeSubTable();

            try {
                secondLevelSubscription->Items = {key};
                this->subTables.insert(secondLevelSubscription, item, key);
            } catch (std::exception& e) {
                log.Error("Subscription error", e);
                onServerError(14, INVALID_SECOND_LEVEL_KEY, key);
                return;
            }

            if (std::dynamic_pointer_cast<ListDescriptor>(this->subFieldDescriptor) != nullptr) {
                secondLevelSubscription->Fields = std::dynamic_pointer_cast<ListDescriptor>(this->subFieldDescriptor)->Original;
            } else {
                secondLevelSubscription->FieldSchema = std::dynamic_pointer_cast<NameDescriptor>(this->subFieldDescriptor)->Original;
            }

            secondLevelSubscription->DataAdapter = this->underDataAdapter;
            secondLevelSubscription->RequestedSnapshot = "yes";
            secondLevelSubscription->requestedMaxFrequency = this->requestedMaxFrequency;

            auto subListener = std::make_shared<SecondLevelSubscriptionListener>(this, item, key);
            secondLevelSubscription->addListener(subListener);

            secondLevelSubscription->setActive();
            this->manager.doAdd(secondLevelSubscription);
        }

        /** Marks this subscription as a sub-table (second-level subscription). */
        void makeSubTable() {
            this->subTableFlag = true;
        }

        /** Checks if this is a sub-table (second-level subscription). */
        bool isSubTable() const {
            // Do not abuse this method
            return this->subTableFlag;
        }

        /** Checks if a sub-table exists for the given item and key. */
        bool hasSubTable(int item, const std::string& key) {
            return this->subTables.find(item) != this->subTables.end() &&
                   this->subTables.at(item).find(key) != this->subTables.at(item).end();
        }

        /** Removes a sub-table (second-level subscription) for the given item and key. */
        void removeSubTable(int item, const std::string& key, bool clean) {
            auto& secondLevelSubscription = this->subTables[item][key];
            secondLevelSubscription->setInactive();
            this->manager.doRemove(*secondLevelSubscription);
            if (clean) {
                this->subTables[item].erase(key);
            }
        }

        /** Removes all sub-tables associated with a given item. */
        void removeItemSubTables(int item) {
            for (auto& keySubPair : this->subTables[item]) {
                this->removeSubTable(item, keySubPair.first, false /* DONT_CLEAN */);
            }
        }

        /** Removes all sub-tables. */
        void removeAllSubTables() {
            for (auto& itemSubTables : this->subTables) {
                for (auto& keySubPair : itemSubTables.second) {
                    this->removeSubTable(itemSubTables.first, keySubPair.first, false /* DONT_CLEAN */);
                }
            }
        }

        /** Sets the size of the second-level schema. */
        void setSecondLevelSchemaSize(int size) {
            this->subFieldDescriptor.setSize(size);
        }

        /** Debugging helper to output the descriptor as a string. */
        void debugDescriptor(const std::string& debugString, const Descriptor& desc) {
            if (/* condition to check if debug is enabled */) {
                std::cout << debugString << (desc ? desc.composedString() : "<null>") << std::endl;
            }
        }

        /** Gets the full size of the schema, including both main and second-level fields. */
        int getFullSchemaSize() const {
            return this->fieldDescriptor.getFullSize();
        }

        /** Gets the size of the main schema only, excluding second-level fields. */
        int getMainSchemaSize() const {
            return this->fieldDescriptor.getSize();
        }

    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTION_HPP
