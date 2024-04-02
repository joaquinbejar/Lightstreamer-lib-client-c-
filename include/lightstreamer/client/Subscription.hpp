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
        static constexpr char const* NO_ITEMS = "Please specify a valid item or item list";
        static constexpr char const* NO_FIELDS = "Invalid Subscription, please specify a field list or field schema";
        static constexpr char const* IS_ALIVE = "Cannot modify an active Subscription, please unsubscribe before applying any change";
        static constexpr char const* NOT_ALIVE = "Subscription is not active";
        static constexpr char const* INVALID_MODE = "The given value is not a valid subscription mode. Admitted values are MERGE, DISTINCT, RAW, COMMAND";
        static constexpr char const* NO_VALID_FIELDS = "Please specify a valid field list";
        static constexpr char const* NO_GROUP_NOR_LIST = "The  item list/item group of this Subscription was not initiated";
        static constexpr char const* NO_SCHEMA_NOR_LIST = "The field list/field schema of this Subscription was not initiated";
        static constexpr char const* MAX_BUF_EXC = "The given value is not valid for this setting; use null, 'unlimited' or a positive integer instead";
        static constexpr char const* NO_SECOND_LEVEL = "Second level field list is only available on COMMAND Subscriptions";
        static constexpr char const* NO_COMMAND = "This method can only be used on COMMAND subscriptions";
        static constexpr char const* NO_SUB_SCHEMA_NOR_LIST = "The second level of this Subscription was not initiated";
        static constexpr char const* RAW_NO_SNAPSHOT = "Snapshot is not permitted if RAW was specified as mode";
        static constexpr char const* NUMERIC_DISTINCT_ONLY = "Numeric values are only allowed when the subscription mode is DISTINCT";
        static constexpr char const* REQ_SNAP_EXC = "The given value is not valid for this setting; use null, 'yes', 'no' or a positive number instead";
        static constexpr char const* ILLEGAL_FREQ_EXC = "Can't change the frequency from/to 'unfiltered' or to null while the Subscription is active";
        static constexpr char const* MAX_FREQ_EXC = "The given value is not valid for this setting; use null, 'unlimited', 'unfiltered' or a positive number instead";
        static constexpr char const* INVALID_SECOND_LEVEL_KEY = "The received key value is not a valid name for an Item";

        static constexpr char const* SIMPLE = "SIMPLE";
        static constexpr char const* METAPUSH = "METAPUSH";
        static constexpr char const* MULTIMETAPUSH = "MULTIMETAPUSH";

        static constexpr char const* OFF = "OFF";
        static constexpr char const* WAITING = "WAITING";
        static constexpr char const* PAUSED = "PAUSED";
        static constexpr char const* SUBSCRIBING = "SUBSCRIBING";
        static constexpr char const* PUSHING = "PUSHING";

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

    public:


    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTION_HPP
