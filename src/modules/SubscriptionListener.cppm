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

export module SubscriptionListener;

import <string>;

namespace Lightstreamer::Cpp::SubscriptionListener {
    typedef int ItemName; // TODO: Define the item name type
    typedef int ItemPos; // TODO: Define the item position type
    typedef int LostUpdates; // TODO: Define the lost updates type
    typedef int Key; // TODO: Define the key type
    typedef int Code; // TODO: Define the code type
    typedef std::string Message; // TODO: Define the message type
    typedef double Frequency; // TODO: Define the frequency type
    typedef std::string Update; // TODO: Define the update type


    /**
    Interface to be implemented to listen to \ref `.Subscription` events comprehending notifications of subscription/unsubscription,
    updates, errors and others.

    Events for these listeners are dispatched by a different thread than the one that generates them.
    This means that, upon reception of an event, it is possible that the internal state of the client has changed.
    On the other hand, all the notifications for a single LightstreamerClient, including notifications to
    \ref `ClientListener`, \ref `SubscriptionListener` and \ref `ClientMessageListener` will be dispatched by the
            same thread.
    **/
    class SubscriptionListener {
    public:

        /**
        Event handler that is called by Lightstreamer each time a request to clear the snapshot pertaining to an item 
        in the Subscription has been received from the Server. More precisely, this kind of request can occur in two
        cases:

        For an item delivered in COMMAND mode, to notify that the state of the item becomes empty; this is
        equivalent to receiving an update carrying a DELETE command once for each key that is currently active.
        For an item delivered in DISTINCT mode, to notify that all the previous updates received for the item should
        be considered as obsolete; hence, if the listener were showing a list of recent updates for the item, it
        should clear the list in order to keep a coherent view.

        Note that, if the involved Subscription has a two-level behavior enabled
        (see \ref `.Subscription.setCommandSecondLevelFields` and \ref `.Subscription.setCommandSecondLevelFieldSchema`)
        , the notification refers to the first-level item (which is in COMMAND mode).
        This kind of notification is not possible for second-level items (which are in MERGE mode).

        @param itemName: name of the involved item. If the Subscription was initialized using an "Item Group" then a
         None value is supplied.
        @param itemPos: 1-based position of the item within the "Item List" or "Item Group".
        **/
        void onClearSnapshot(ItemName itemname, ItemPos itempos) {}

        /**
        Event handler that is called by Lightstreamer to notify that, due to internal resource limitations,
        Lightstreamer Server dropped one or more updates for an item that was subscribed to as a second-level
        subscription.
        Such notifications are sent only if the Subscription was configured in unfiltered mode (second-level items are
        always in "MERGE" mode and inherit the frequency configuration from the first-level Subscription).

        By implementing this method it is possible to perform recovery actions.

        @param lostUpdates: The number of consecutive updates dropped for the item.
        @param key: The value of the key that identifies the second-level item.

        .. seealso:: \ref `.Subscription.setRequestedMaxFrequency`
        .. seealso:: \ref `.Subscription.setCommandSecondLevelFields`
        .. seealso:: \ref `.Subscription.setCommandSecondLevelFieldSchema`
        **/
        void onCommandSecondLevelItemLostUpdates(LostUpdates lostupdates, Key key) {}

        /**
        Event handler that is called when the Server notifies an error on a second-level subscription.

        By implementing this method it is possible to perform recovery actions.

        @param code: The error code sent by the Server. It can be one of the following:

        * 14 - the key value is not a valid name for the Item to be subscribed; only in this case, the error is
        detected directly by the library before issuing the actual request to the Server
        * 17 - bad Data Adapter name or default Data Adapter not defined for the current Adapter Set
        * 21 - bad Group name
        * 22 - bad Group name for this Schema
        * 23 - bad Schema name
        * 24 - mode not allowed for an Item
        * 26 - unfiltered dispatching not allowed for an Item, because a frequency limit is associated to the item
        * 27 - unfiltered dispatching not supported for an Item, because a frequency prefiltering is applied for the item
        * 28 - unfiltered dispatching is not allowed by the current license terms (for special licenses only)
        * 66 - an unexpected exception was thrown by the Metadata Adapter while authorizing the connection
        * 68 - the Server could not fulfill the request because of an internal error.
        * <= 0 - the Metadata Adapter has refused the subscription or unsubscription request; the code value is
         dependent on the specific Metadata Adapter implementation

        @param message: The description of the error sent by the Server; it can be None.
        @param key: The value of the key that identifies the second-level item.

        .. seealso:: \ref `.ConnectionDetails.setAdapterSet`
        .. seealso:: \ref `.Subscription.setCommandSecondLevelFields`
        .. seealso:: \ref `.Subscription.setCommandSecondLevelFieldSchema`
        **/
        void onCommandSecondLevelSubscriptionError(Code code, Message message, Key key) {}

        /**
        Event handler that is called by Lightstreamer to notify that all snapshot events for an item in the
        Subscription have been received, so that real time events are now going to be received. The received
        snapshot could be empty. Such notifications are sent only if the items are delivered in DISTINCT or COMMAND
                subscription mode and snapshot information was indeed requested for the items. By implementing this
        method it is possible to perform actions which require that all the initial values have been received.

        Note that, if the involved Subscription has a two-level behavior enabled
        (see \ref `.Subscription.setCommandSecondLevelFields` and \ref `.Subscription.setCommandSecondLevelFieldSchema`)
        , the notification refers to the first-level item (which is in COMMAND mode).
        Snapshot-related updates for the second-level items
                (which are in MERGE mode) can be received both before and after this notification.

        @param itemName: name of the involved item. If the Subscription was initialized using an "Item Group" then a
        None value is supplied.
        @param itemPos: 1-based position of the item within the "Item List" or "Item Group".

        .. seealso:: \ref `.Subscription.setRequestedSnapshot`
        .. seealso:: \ref `ItemUpdate.isSnapshot`
        **/
        void onEndOfSnapshot(ItemName itemname, ItemPos itempos) {}

        /**
        Event handler that is called by Lightstreamer to notify that, due to internal resource limitations,
        Lightstreamer Server dropped one or more updates for an item in the Subscription.
        Such notifications are sent only if the items are delivered in an unfiltered mode; this occurs if the
                subscription mode is:

                * RAW
                * MERGE or DISTINCT, with unfiltered dispatching specified
                * COMMAND, with unfiltered dispatching specified
                * COMMAND, without unfiltered dispatching specified (in this case, notifications apply to ADD and DELETE
                * events only)

        By implementing this method it is possible to perform recovery actions.

        @param itemName: name of the involved item. If the Subscription was initialized using an "Item Group" then a
         None value is supplied.
        @param itemPos: 1-based position of the item within the "Item List" or "Item Group".
        @param lostUpdates: The number of consecutive updates dropped for the item.

        .. seealso:: \ref `.Subscription.setRequestedMaxFrequency`
        **/
        void onItemLostUpdates(ItemName itemname, ItemPos itempos, LostUpdates lostupdates) {}

        /**
        Event handler that is called by Lightstreamer each time an update pertaining to an item in the Subscription
        has been received from the Server.

        @param itemUpdate: a value object containing the updated values for all the fields, together with
         meta-information about the update itself and some helper methods that can be used to iterate through all or
         new values.
        **/
        void onItemUpdate(Update update) {}

        /**
        Event handler that receives a notification when the SubscriptionListener instance is removed from a Subscription
        through \ref `.Subscription.removeListener`. This is the last event to be fired on the listener.
        **/
        void onListenEnd() {}

        /**
         * Event handler that receives a notification when the SubscriptionListener instance is added to a Subscription
         * through \ref `.Subscription.addListener`. This is the first event to be fired on the listener.
        **/
        void onListenStart() {}

        /**
        Event handler that is called by Lightstreamer to notify the client with the real maximum update frequency of
        the Subscription.
        It is called immediately after the Subscription is established and in response to a requested change
        (see \ref `.Subscription.setRequestedMaxFrequency`).
        Since the frequency limit is applied on an item basis and a Subscription can involve multiple items,
        this is actually the maximum frequency among all items. For Subscriptions with two-level behavior
        (see \ref `.Subscription.setCommandSecondLevelFields` and \ref `.Subscription.setCommandSecondLevelFieldSchema`)
        , the reported frequency limit applies to both first-level and second-level items.

        The value may differ from the requested one because of restrictions operated on the server side,
                but also because of number rounding.

        Note that a maximum update frequency (that is, a non-unlimited one) may be applied by the Server
        even when the subscription mode is RAW or the Subscription was done with unfiltered dispatching.

        @param frequency:  A decimal number, representing the maximum frequency applied by the Server (expressed in updates per second), or the string "unlimited". A None value is possible in rare cases, when the frequency can no longer be determined.
        **/
        void onRealMaxFrequency(Frequency frequency) {}

        /**
        Event handler that is called by Lightstreamer to notify that a Subscription has been successfully subscribed
        to through the Server. This can happen multiple times in the life of a Subscription instance, in case the
                Subscription is performed multiple times through \ref `.LightstreamerClient.unsubscribe` and
        \ref `.LightstreamerClient.subscribe`. This can also happen multiple times in case of automatic
        recovery after a connection restart.

        This notification is always issued before the other ones related to the same subscription. It invalidates all
                data that has been received previously.

        Note that two consecutive calls to this method are not possible, as before a second onSubscription event is
                fired an \ref `onUnsubscription` event is eventually fired.

        If the involved Subscription has a two-level behavior enabled
        (see \ref `.Subscription.setCommandSecondLevelFields` and
         \ref `.Subscription.setCommandSecondLevelFieldSchema`), second-level subscriptions are not notified.
        **/
        void onSubscription() {}

        /**
         Event handler that is called when the Server notifies an error on a Subscription. By implementing this method it
        is possible to perform recovery actions.

        Note that, in order to perform a new subscription attempt, \ref `.LightstreamerClient.unsubscribe`
        and \ref `.LightstreamerClient.subscribe` should be issued again, even if no change to the Subscription
                attributes has been applied.

        @param code: The error code sent by the Server. It can be one of the following:

                * 15 - "key" field not specified in the schema for a COMMAND mode subscription
        * 16 - "command" field not specified in the schema for a COMMAND mode subscription
        * 17 - bad Data Adapter name or default Data Adapter not defined for the current Adapter Set
        * 21 - bad Group name
        * 22 - bad Group name for this Schema
        * 23 - bad Schema name
        * 24 - mode not allowed for an Item
        * 25 - bad Selector name
        * 26 - unfiltered dispatching not allowed for an Item, because a frequency limit is associated to the item
        * 27 - unfiltered dispatching not supported for an Item, because a frequency prefiltering is applied for the item
        * 28 - unfiltered dispatching is not allowed by the current license terms (for special licenses only)
        * 29 - RAW mode is not allowed by the current license terms (for special licenses only)
        * 30 - subscriptions are not allowed by the current license terms (for special licenses only)
        * 66 - an unexpected exception was thrown by the Metadata Adapter while authorizing the connection
        * 68 - the Server could not fulfill the request because of an internal error.
        * <= 0 - the Metadata Adapter has refused the subscription or unsubscription request; the code value is dependent on the specific Metadata Adapter implementation

        @param message: The description of the error sent by the Server; it can be None.

        .. seealso:: \ref `.ConnectionDetails.setAdapterSet`
        **/
        void onSubscriptionError(Code code, Message message, Key key) {}

        /**
        Event handler that is called by Lightstreamer to notify that a Subscription has been successfully unsubscribed
        from. This can happen multiple times in the life of a Subscription instance, in case the Subscription is
        performed multiple times through \ref `.LightstreamerClient.unsubscribe` and
        \ref `.LightstreamerClient.subscribe`. This can also happen multiple times in case of automatic
        recovery after a connection restart.

        After this notification no more events can be received until a new #onSubscription event.

        Note that two consecutive calls to this method are not possible, as before a second onUnsubscription event
        is fired an \ref `onSubscription` event is eventually fired.

        If the involved Subscription has a two-level behavior enabled
        (see \ref `.Subscription.setCommandSecondLevelFields` and
         \ref `.Subscription.setCommandSecondLevelFieldSchema`)
        , second-level unsubscriptions are not notified.
        **/
        void onUnsubscription() {}

    };
}