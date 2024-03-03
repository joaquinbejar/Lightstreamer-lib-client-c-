/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 2/3/24
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

export module Subscription;
namespace Lightstreamer::Cpp::Subscription {
    typedef int Listener; // TODO: Define the listener type
    typedef int ItemNameOrPos; // TODO: Define the data ItemNameOrPos type
    typedef int KeyValue; // TODO: Define the data KeyValue type
    typedef int FieldNameOrPos; // TODO: Define the data FieldNameOrPos type
    typedef int DataAdapter; // TODO: Define the data DataAdapter type
    typedef int Fields; // TODO: Define the data Fields type
    typedef int Schema; // TODO: Define the data Schema type
    typedef int Group; // TODO: Define the data Group type
    typedef int Items; // TODO: Define the data Items type
    typedef int Size; // TODO: Define the data Size type
    typedef int Frequency; // TODO: Define the data Frequency type
    typedef int Snapshot; // TODO: Define the data Snapshot type
    typedef int Selector; // TODO: Define the data Selector type

    /**
    Class representing a Subscription to be submitted to a Lightstreamer Server. It contains subscription details and 
     the listeners needed to process the real-time data. 

    After the creation, a Subscription object is in the "inactive" state. When a Subscription object is subscribed to 
    on a LightstreamerClient object, through the \ref `LightstreamerClient.subscribe` method, its state becomes "active".
    This means that the client activates a subscription to the required items through
            Lightstreamer Server and the Subscription object begins to receive real-time events.

    A Subscription can be configured to use either an Item Group or an Item List to specify the
    items to be subscribed to and using either a Field Schema or Field List to specify the fields.

    "Item Group" and "Item List" are defined as follows:

    * "Item Group": an Item Group is a String identifier representing a list of items. Such Item Group has to be 
    expanded into a list of items by the getItems method of the MetadataProvider of the associated Adapter Set. 
    When using an Item Group, items in the subscription are identified by their 1-based index within the group. It is 
    possible to configure the Subscription to use an "Item Group" using the \ref `setItemGroup` method.
    * "Item List": an Item List is an array of Strings each one representing an item. For the Item List to be correctly 
    interpreted a LiteralBasedProvider or a MetadataProvider with a compatible implementation of getItems has to be 
    configured in the associated Adapter Set. Note that no item in the list can be empty, can contain spaces or can be
    a number. When using an Item List, items in the subscription are identified by their name or by their 1-based index
     within the list. It is possible to configure the Subscription to use an "Item List" using the \ref `setItems` 
     method or by specifying it in the constructor.

    "Field Schema" and "Field List" are defined as follows:

    * "Field Schema": a Field Schema is a String identifier representing a list of fields. Such Field Schema has to be 
    expanded into a list of fields by the getFields method of the MetadataProvider of the associated Adapter Set. When 
    using a Field Schema, fields in the subscription are identified by their 1-based index within the schema. It is 
    possible to configure the Subscription to use a "Field Schema" using the \ref `setFieldSchema` method.
    * "Field List": a Field List is an array of Strings each one representing a field. For the Field List to be 
    correctly interpreted a LiteralBasedProvider or a MetadataProvider with a compatible implementation of getFields 
    has to be configured in the associated Adapter Set. Note that no field in the list can be empty or can contain 
    spaces. When using a Field List, fields in the subscription are identified by their name or by their 1-based index 
    within the list. It is possible to configure the Subscription to use a "Field List" using the \ref `setFields` 
    method or by specifying it in the constructor.

    A Subscription can be supplied to \ref `LightstreamerClient.subscribe` and \ref `LightstreamerClient.unsubscribe`,
    in order to bring the Subscription to "active" or back to "inactive" state.

    Note that all of the methods used to describe the subscription to the server can only be called while the instance 
    is in the "inactive" state; the only exception is \ref `setRequestedMaxFrequency`.

    @param subscriptionMode: the subscription mode for the items, required by Lightstreamer Server. Permitted values are:

    * MERGE
            * DISTINCT
    * RAW
            * COMMAND

    @param items: an array of items to be subscribed to through Lightstreamer server. It is also possible specify the 
     "Item List" or "Item Group" later through \ref `setItems` and \ref `setItemGroup`.

    @param fields: an array of fields for the items to be subscribed to through Lightstreamer Server. It is also 
     possible to specify the "Field List" or "Field Schema" later through \ref `setFields` and \ref `setFieldSchema`.

    @raises IllegalArgumentException: If no or invalid subscription mode is passed.
    @raises IllegalArgumentException: If either the items or the fields array is left None.
    @raises IllegalArgumentException: If the specified "Item List" or "Field List" is not valid; see \ref `setItems` 
     and \ref `setFields` for details.
    **/
    class Subscription {
    public:

        /**
		Adds a listener that will receive events from the Subscription instance.

        The same listener can be added to several different Subscription instances.

        @b lifecycle A listener can be added at any time. A call to add a listener already
                present will be ignored.

        @param listener: An object that will receive the events as documented in the SubscriptionListener interface.

        .. seealso:: \ref `removeListener`
        **/
        void addListener(Listener listener) {}

        /**
		Returns the position of the "command" field in a COMMAND Subscription.

        This method can only be used if the Subscription mode is COMMAND and the Subscription
                was initialized using a "Field Schema".

        @b lifecycle This method can be called at any time after the first
                \ref `.SubscriptionListener.onSubscription` event.

        @raises IllegalStateException: if the Subscription mode is not COMMAND or if the
         \ref `.SubscriptionListener.onSubscription` event for this Subscription was not yet fired.
        @raises IllegalStateException: if a "Field List" was specified.

        @return: the 1-based position of the "command" field within the "Field Schema".
        **/
        void getCommandPosition() {}

        /**
		Inquiry method that can be used to read the second-level Data Adapter name configured
        through \ref `setCommandSecondLevelDataAdapter`.

        @b lifecycle This method can be called at any time.

        @raises IllegalStateException: if the Subscription mode is not COMMAND
        @return: the name of the second-level Data Adapter.

        .. seealso:: \ref `setCommandSecondLevelDataAdapter`
        **/
        void getCommandSecondLevelDataAdapter() {}

        /**
		Inquiry method that can be used to read the "Field List" specified for second-level
        Subscriptions.

        @b lifecycle This method can only be called if the second-level of this Subscription
                has been initialized using a "Field List"

        @raises IllegalStateException: if the Subscription mode is not COMMAND
        @return: the list of fields to be subscribed to through the server, or None if the Subscription was initialized with a "Field Schema" or was not initialized at all.

        .. seealso:: \ref `Subscription.setCommandSecondLevelFields`
        **/
        void getCommandSecondLevelFields() {}

        /**
		Inquiry method that can be used to read the "Field Schema" specified for second-level
        Subscriptions.

        @b lifecycle This method can only be called if the second-level of this Subscription has
        been initialized using a "Field Schema".

        @raises IllegalStateException: if the Subscription mode is not COMMAND
        @return: the "Field Schema" to be subscribed to through the server, or None if the Subscription was initialized with a "Field List" or was not initialized at all.

        .. seealso:: \ref `Subscription.setCommandSecondLevelFieldSchema`
        **/
        void getCommandSecondLevelFieldSchema() {}

        /**
		Returns the latest value received for the specified item/key/field combination. This method can only be used if the Subscription mode is COMMAND. Subscriptions with two-level behavior are also supported, hence the specified field can be either a first-level or a second-level one.

        It is suggested to consume real-time data by implementing and adding a proper :class:`.SubscriptionListener` rather than probing this method.

        Note that internal data is cleared when the Subscription is unsubscribed from.

        @b lifecycle This method can be called at any time; if called to retrieve a value that has not been received yet, then it will return None.

        @raises IllegalArgumentException: if an invalid item name or field name is specified or if the specified item position or field position is out of bounds.
        @raises IllegalStateException: if the Subscription mode is not COMMAND.

        @param itemIdentifier: a String representing an item in the configured item list or a Number representing the 1-based position of the item in the specified item group. (In case an item list was specified, passing the item position is also possible).

        @param keyValue: a String containing the value of a key received on the COMMAND subscription.

        @param fieldIdentifier: a String representing a field in the configured field list or a Number representing the 1-based position of the field in the specified field schema. (In case a field list was specified, passing the field position is also possible).

        @return: the current value for the specified field of the specified key within the specified item (possibly None), or None if the specified key has not been added yet (note that it might have been added and eventually deleted).
        **/
        void getCommandValue(ItemNameOrPos itemnameorpos, KeyValue keyvalue, FieldNameOrPos fieldnameorpos) {}

        /**
		Inquiry method that can be used to read the name of the Data Adapter specified for this
        Subscription through \ref `setDataAdapter`.
        @b lifecycle This method can be called at any time.

        @return: the name of the Data Adapter; returns None if no name has been configured, so that the "DEFAULT" Adapter Set is used.
        **/
        void getDataAdapter() {}

        /**
		Inquiry method that can be used to read the "Field List" specified for this Subscription.

        @b lifecycle  This method can only be called if the Subscription has been initialized
        using a "Field List".

        @return: the "Field List" to be subscribed to through the server, or None if the Subscription was initialized with a "Field Schema" or was not initialized at all.
        **/
        void getFields() {}

        /**
		Inquiry method that can be used to read the field schema specified for this Subscription.

        @b lifecycle This method can only be called if the Subscription has been initialized
        using a "Field Schema"

        @return: the "Field Schema" to be subscribed to through the server, or None if the Subscription was initialized with a "Field List" or was not initialized at all.
        **/
        void getFieldSchema() {}

        /**
		Inquiry method that can be used to read the item group specified for this Subscription.

        @b lifecycle This method can only be called if the Subscription has been initialized
        using an "Item Group"

        @return: the "Item Group" to be subscribed to through the server, or None if the Subscription was initialized with an "Item List" or was not initialized at all.
        **/
        void getItemGroup() {}

        /**
		Inquiry method that can be used to read the "Item List" specified for this Subscription.
        Note that if the single-item-constructor was used, this method will return an array
        of length 1 containing such item.

        @b lifecycle This method can only be called if the Subscription has been initialized
                with an "Item List".

        @return: the "Item List" to be subscribed to through the server, or None if the Subscription was initialized with an "Item Group" or was not initialized at all.
        **/
        void getItems() {}

        /**
		Returns the position of the "key" field in a COMMAND Subscription.

        This method can only be used if the Subscription mode is COMMAND
                and the Subscription was initialized using a "Field Schema".

        @b lifecycle This method can be called at any time.

        @raises IllegalStateException: if the Subscription mode is not COMMAND or if the \ref `.SubscriptionListener.onSubscription` event for this Subscription was not yet fired.

        @return: the 1-based position of the "key" field within the "Field Schema".
        **/
        void getKeyPosition() {}

        /**
		Returns a list containing the :class:`.SubscriptionListener` instances that were
        added to this client.

        @return: a list containing the listeners that were added to this client.

        .. seealso:: \ref `addListener`
        **/
        void getListeners() {}

        /**
		Inquiry method that can be used to read the mode specified for this
        Subscription.

        @b lifecycle This method can be called at any time.

        @return: the Subscription mode specified in the constructor.
        **/
        void getMode() {}

        /**
		Inquiry method that can be used to read the buffer size, configured though
        \ref `setRequestedBufferSize`, to be requested to the Server for
        this Subscription.

        @b lifecycle This method can be called at any time.

        @return:  An integer number, representing the buffer size to be requested to the server, or the string "unlimited", or None.
        **/
        void getRequestedBufferSize() {}

        /**
		Inquiry method that can be used to read the max frequency, configured
        through \ref `setRequestedMaxFrequency`, to be requested to the
                Server for this Subscription.

        @b lifecycle This method can be called at any time.

        @return:  A decimal number, representing the max frequency to be requested to the server (expressed in updates per second), or the strings "unlimited" or "unfiltered", or None.
        **/
        void getRequestedMaxFrequency() {}

        /**
		Inquiry method that can be used to read the snapshot preferences,
        configured through \ref `setRequestedSnapshot`, to be requested
                to the Server for this Subscription.

        @b lifecycle This method can be called at any time.

        @return:  "yes", "no", None, or an integer number.
        **/
        void getRequestedSnapshot() {}

        /**
		Inquiry method that can be used to read the selector name
        specified for this Subscription through \ref `setSelector`.

        @b lifecycle This method can be called at any time.

        @return: the name of the selector.
        **/
        void getSelector() {}

        /**
		Returns the latest value received for the specified item/field pair.

        It is suggested to consume real-time data by implementing and adding
                a proper :class:`.SubscriptionListener` rather than probing this method.
        In case of COMMAND Subscriptions, the value returned by this
        method may be misleading, as in COMMAND mode all the keys received, being
                part of the same item, will overwrite each other; for COMMAND Subscriptions,
                use \ref `Subscription.getCommandValue` instead.

        Note that internal data is cleared when the Subscription is
        unsubscribed from.

        @b lifecycle This method can be called at any time; if called
                to retrieve a value that has not been received yet, then it will return None.

        @raises IllegalArgumentException: if an invalid item name or field name is specified or if the specified item position or field position is out of bounds.

        @param itemNameOrPos: a String representing an item in the configured item list or a Number representing the 1-based position of the item in the specified item group. (In case an item list was specified, passing the item position is also possible).

        @param fieldNameOrPos: a String representing a field in the configured field list or a Number representing the 1-based position of the field in the specified field schema. (In case a field list was specified, passing the field position is also possible).

        @return: the current value for the specified field of the specified item(possibly None), or None if no value has been received yet.
        **/
        void getValue(ItemNameOrPos itemnameorpos, FieldNameOrPos fieldnameorpos) {}

        /**
		Inquiry method that checks if the Subscription is currently "active" or not.Most of the Subscription properties cannot be modified if a Subscription is "active".

        The status of a Subscription is changed to "active" through the
                \ref `LightstreamerClient.subscribe` method and back to
        "inactive" through the \ref `LightstreamerClient.unsubscribe` one.

        @b lifecycle This method can be called at any time.

        @return: true/false if the Subscription is "active" or not.

        .. seealso:: \ref `LightstreamerClient.subscribe`
        .. seealso:: \ref `LightstreamerClient.unsubscribe`
        **/
        void isActive() {}

        /**
		Inquiry method that checks if the Subscription is currently subscribed to
        through the server or not.

        This flag is switched to true by server sent Subscription events, and
        back to false in case of client disconnection,
        \ref `LightstreamerClient.unsubscribe` calls and server
        sent unsubscription events.

        @b lifecycle This method can be called at any time.

        @return: true/false if the Subscription is subscribed to through the server or not.
        **/
        void isSubscribed() {}

        /**
		Removes a listener from the Subscription instance so that it will not receive
        events anymore.

        @b lifecycle a listener can be removed at any time.

        @param listener: The listener to be removed.

        .. seealso:: \ref `addListener`
        **/
        void removeListener(Listener listener) {}

        /**
		Setter method that sets the name of the second-level Data Adapter (within
        the Adapter Set used by the current session) that supplies all the
        second-level items.

        All the possible second-level items should be supplied in "MERGE" mode
                with snapshot available.

        The Data Adapter name is configured on the server side through the
        "name" attribute of the <data_provider> element, in the "adapters.xml"
        file that defines the Adapter Set (a missing attribute configures the
        "DEFAULT" name).

        **default** The default Data Adapter for the Adapter Set,
        configured as "DEFAULT" on the Server.

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalStateException: if the Subscription is currently "active".
        @raises IllegalStateException: if the Subscription mode is not "COMMAND".

        @param dataAdapter: the name of the Data Adapter. A None value is equivalent to the "DEFAULT" name.

        .. seealso:: \ref `Subscription.setCommandSecondLevelFields`
        .. seealso:: \ref `Subscription.setCommandSecondLevelFieldSchema`
        **/
        void setCommandSecondLevelDataAdapter(DataAdapter dataadapter) {}

        /**
		Setter method that sets the "Field List" to be subscribed to through
        Lightstreamer Server for the second-level items. It can only be used on
        COMMAND Subscriptions.

        Any call to this method will override any "Field List" or "Field Schema"
        previously specified for the second-level.

        Calling this method enables the two-level behavior:

                in synthesis, each time a new key is received on the COMMAND Subscription,
        the key value is treated as an Item name and an underlying Subscription for
        this Item is created and subscribed to automatically, to feed fields specified
                by this method. This mono-item Subscription is specified through an "Item List"
        containing only the Item name received. As a consequence, all the conditions
                provided for subscriptions through Item Lists have to be satisfied. The item is
                subscribed to in "MERGE" mode, with snapshot request and with the same maximum
                frequency setting as for the first-level items (including the "unfiltered"
        case). All other Subscription properties are left as the default. When the
        key is deleted by a DELETE command on the first-level Subscription, the
        associated second-level Subscription is also unsubscribed from.

        Specifying None as parameter will disable the two-level behavior.

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalArgumentException: if any of the field names in the "Field List" contains a space or is empty/None.
        @raises IllegalStateException: if the Subscription is currently "active".
        @raises IllegalStateException: if the Subscription mode is not "COMMAND".

        @param fields: An array of Strings containing a list of fields to be subscribed to through the server. Ensure that no name conflict is generated between first-level and second-level fields. In case of conflict, the second-level field will not be accessible by name, but only by position.

        .. seealso:: \ref `Subscription.setCommandSecondLevelFieldSchema`
        **/
        void setCommandSecondLevelFields(Fields fields) {}

        /**
		Setter method that sets the "Field Schema" to be subscribed to through
        Lightstreamer Server for the second-level items. It can only be used on
        COMMAND Subscriptions.

        Any call to this method will override any "Field List" or "Field Schema"
        previously specified for the second-level.

        Calling this method enables the two-level behavior:

                in synthesis, each time a new key is received on the COMMAND Subscription,
        the key value is treated as an Item name and an underlying Subscription for
        this Item is created and subscribed to automatically, to feed fields specified
                by this method. This mono-item Subscription is specified through an "Item List"
        containing only the Item name received. As a consequence, all the conditions
                provided for subscriptions through Item Lists have to be satisfied. The item is
                subscribed to in "MERGE" mode, with snapshot request and with the same maximum
                frequency setting as for the first-level items (including the "unfiltered"
        case). All other Subscription properties are left as the default. When the
        key is deleted by a DELETE command on the first-level Subscription, the
        associated second-level Subscription is also unsubscribed from.

        Specify None as parameter will disable the two-level behavior.

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalStateException: if the Subscription is currently "active".
        @raises IllegalStateException: if the Subscription mode is not "COMMAND".

        @param schemaName: A String to be expanded into a field list by the Metadata Adapter.

        .. seealso:: \ref `Subscription.setCommandSecondLevelFields`
        **/
        void setCommandSecondLevelFieldSchema(Schema schema) {}

        /**
		Setter method that sets the name of the Data Adapter
        (within the Adapter Set used by the current session)
        that supplies all the items for this Subscription.

        The Data Adapter name is configured on the server side through
                the "name" attribute of the "data_provider" element, in the
        "adapters.xml" file that defines the Adapter Set (a missing attribute
        configures the "DEFAULT" name).

        Note that if more than one Data Adapter is needed to supply all the
                items in a set of items, then it is not possible to group all the
                items of the set in a single Subscription. Multiple Subscriptions
        have to be defined.

        **default** The default Data Adapter for the Adapter Set,
        configured as "DEFAULT" on the Server.

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalStateException: if the Subscription is currently "active".

        @param dataAdapter: the name of the Data Adapter. A None value is equivalent to the "DEFAULT" name.

        .. seealso:: \ref `ConnectionDetails.setAdapterSet`
        **/
        void setDataAdapter(DataAdapter dataadapter) {}

        /**
		Setter method that sets the "Field List" to be subscribed to through
        Lightstreamer Server.

        Any call to this method will override any "Field List" or "Field Schema"
        previously specified.

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalArgumentException: if any of the field names in the list contains a space or is empty/None.
        @raises IllegalStateException: if the Subscription is currently "active".

        @param fields: an array of fields to be subscribed to through the server.
        **/
        void setFields(Fields fields) {}

        /**
		Setter method that sets the "Field Schema" to be subscribed to through
        Lightstreamer Server.

        Any call to this method will override any "Field List" or "Field Schema"
        previously specified.

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalStateException: if the Subscription is currently "active".

        @param schemaName: A String to be expanded into a field list by the Metadata Adapter.
        **/
        void setFieldSchema(Schema schema) {}

        /**
		Setter method that sets the "Item Group" to be subscribed to through
        Lightstreamer Server.

        Any call to this method will override any "Item List" or "Item Group"
        previously specified.

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalStateException: if the Subscription is currently "active".

        @param groupName: A String to be expanded into an item list by the Metadata Adapter.
        **/
        void setItemGroup(Group group) {}

        /**
		Setter method that sets the "Item List" to be subscribed to through
        Lightstreamer Server.

        Any call to this method will override any "Item List" or "Item Group"
        previously specified.

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalArgumentException: if any of the item names in the "Item List" contains a space or is a number or is empty/None.
        @raises IllegalStateException: if the Subscription is currently "active".

        @param items: an array of items to be subscribed to through the server.
        **/
        void setItems(Items items) {}

        /**
		Setter method that sets the length to be requested to Lightstreamer
        Server for the internal queuing buffers for the items in the Subscription.
        A Queuing buffer is used by the Server to accumulate a burst
        of updates for an item, so that they can all be sent to the client,
        despite of bandwidth or frequency limits. It can be used only when the
                subscription mode is MERGE or DISTINCT and unfiltered dispatching has
        not been requested. Note that the Server may pose an upper limit on the
                size of its internal buffers.

        **default** None, meaning to lean on the Server default based on the subscription
        mode. This means that the buffer size will be 1 for MERGE
                subscriptions and "unlimited" for DISTINCT subscriptions. See
                the "General Concepts" document for further details.

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalStateException: if the Subscription is currently "active".
        @raises IllegalArgumentException: if the specified value is not None nor "unlimited" nor a valid positive integer number.

        @param size:  An integer number, representing the length of the internal queuing buffers to be used in the Server. If the string "unlimited" is supplied, then no buffer size limit is requested (the check is case insensitive). It is also possible to supply a None value to stick to the Server default (which currently depends on the subscription mode).

        .. seealso:: \ref `Subscription.setRequestedMaxFrequency`
        **/
        void setRequestedBufferSize(Size size) {}

        /**
		Setter method that sets the maximum update frequency to be requested to
        Lightstreamer Server for all the items in the Subscription. It can
        be used only if the Subscription mode is MERGE, DISTINCT or
        COMMAND (in the latter case, the frequency limitation applies to the
        UPDATE events for each single key). For Subscriptions with two-level behavior
                (see \ref `Subscription.setCommandSecondLevelFields` and \ref `Subscription.setCommandSecondLevelFieldSchema`)
        , the specified frequency limit applies to both first-level and second-level items.

        Note that frequency limits on the items can also be set on the
                server side and this request can only be issued in order to furtherly
                reduce the frequency, not to rise it beyond these limits.

        This method can also be used to request unfiltered dispatching
        for the items in the Subscription. However, unfiltered dispatching
        requests may be refused if any frequency limit is posed on the server
        side for some item.

        **general edition note** A further global frequency limit could also be imposed by the Server,
        depending on Edition and License Type; this specific limit also applies to RAW mode and
                to unfiltered dispatching.
        To know what features are enabled by your license, please see the License tab of the
                Monitoring Dashboard (by default, available at /dashboard).

        **default** None, meaning to lean on the Server default based on the subscription
        mode. This consists, for all modes, in not applying any frequency
                limit to the subscription (the same as "unlimited"); see the "General Concepts"
        document for further details.

        @b lifecycle This method can can be called at any time with some
        differences based on the Subscription status:

                * If the Subscription instance is in its "inactive" state then this method can be called at will.
        * If the Subscription instance is in its "active" state then the method can still be called unless the current value is "unfiltered" or the supplied value is "unfiltered" or None. If the Subscription instance is in its "active" state and the connection to the server is currently open, then a request to change the frequency of the Subscription on the fly is sent to the server.

        @raises IllegalStateException: if the Subscription is currently "active" and the current value of this property is "unfiltered".
        @raises IllegalStateException: if the Subscription is currently "active" and the given parameter is None or "unfiltered".
        @raises IllegalArgumentException: if the specified value is not None nor one of the special "unlimited" and "unfiltered" values nor a valid positive number.

        @param freq:  A decimal number, representing the maximum update frequency (expressed in updates per second) for each item in the Subscription; for instance, with a setting of 0.5, for each single item, no more than one update every 2 seconds will be received. If the string "unlimited" is supplied, then no frequency limit is requested. It is also possible to supply the string "unfiltered", to ask for unfiltered dispatching, if it is allowed for the items, or a None value to stick to the Server default (which currently corresponds to "unlimited"). The check for the string constants is case insensitive.
        **/
        void setRequestedMaxFrequency(Frequency frequency) {}

        /**
		Setter method that enables/disables snapshot delivery request for the
        items in the Subscription. The snapshot can be requested only if the
                Subscription mode is MERGE, DISTINCT or COMMAND.

        **default** "yes" if the Subscription mode is not "RAW",
        None otherwise.

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalStateException: if the Subscription is currently "active".
        @raises IllegalArgumentException: if the specified value is not "yes" nor "no" nor None nor a valid integer positive number.
        @raises IllegalArgumentException: if the specified value is not compatible with the mode of the Subscription:

                * In case of a RAW Subscription only None is a valid value;
        * In case of a non-DISTINCT Subscription only None "yes" and "no" are valid values.


        @param required: "yes"/"no" to request/not request snapshot delivery (the check is case insensitive). If the Subscription mode is DISTINCT, instead of "yes", it is also possible to supply an integer number, to specify the requested length of the snapshot (though the length of the received snapshot may be less than requested, because of insufficient data or server side limits); passing "yes"  means that the snapshot length should be determined only by the Server. None is also a valid value; if specified, no snapshot preference will be sent to the server that will decide itself whether or not to send any snapshot.

        .. seealso:: \ref `.ItemUpdate.isSnapshot`
        **/
        void setRequestedSnapshot(Snapshot snapshot) {}

        /**
		Setter method that sets the selector name for all the items in the
        Subscription. The selector is a filter on the updates received. It is
        executed on the Server and implemented by the Metadata Adapter.

        **default** None (no selector).

        @b lifecycle This method can only be called while the Subscription
        instance is in its "inactive" state.

        @raises IllegalStateException: if the Subscription is currently "active".

        @param selector: name of a selector, to be recognized by the Metadata Adapter, or None to unset the selector.
        **/
        void setSelector(Selector selector) {}


    };
}