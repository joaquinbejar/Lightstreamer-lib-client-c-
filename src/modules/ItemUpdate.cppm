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

export module ItemUpdate;
namespace Lightstreamer::Cpp::ItemUpdate {
    typedef int FieldNameOrPos; // TODO: Define FieldNameOrPos type

    /*
    Contains all the information related to an update of the field values for an item.
    It reports all the new values of the fields.

    **COMMAND Subscription**

    If the involved Subscription is a COMMAND Subscription, then the values for the current
    update are meant as relative to the same key.

    Moreover, if the involved Subscription has a two-level behavior enabled, then each update
            may be associated with either a first-level or a second-level item. In this case, the reported
    fields are always the union of the first-level and second-level fields and each single update
            can only change either the first-level or the second-level fields (but for the "command" field,
    which is first-level and is always set to "UPDATE" upon a second-level update); note
            that the second-level field values are always None until the first second-level update
    occurs). When the two-level behavior is enabled, in all methods where a field name has to
    be supplied, the following convention should be followed:

    * The field name can always be used, both for the first-level and the second-level fields. In case of name conflict,
    the first-level field is meant.
    * The field position can always be used; however, the field positions for the second-level fields start at the
    highest position of the first-level field list + 1. If a field schema had been specified for either first-level or
    second-level Subscriptions, then client-side knowledge of the first-level schema length would be required.
    */
    class ItemUpdate {
    public:

        /*
		Returns a map containing the values for each field changed with the last server update.
        The related field name is used as key for the values in the map.
        Note that if the Subscription mode of the involved Subscription is COMMAND, then changed fields
        are meant as relative to the previous update for the same key. On such tables if a DELETE command
        is received, all the fields, excluding the key field, will be present as changed, with None value.
        All of this is also true on tables that have the two-level behavior enabled, but in case of
        DELETE commands second-level fields will not be iterated.

        :raises IllegalStateException: if the Subscription was initialized using a field schema.

        :return: A map containing the values for each field changed with the last server update.

        .. seealso:: :meth:`.Subscription.setFieldSchema`
        .. seealso:: :meth:`.Subscription.setFields`
        */
        void getChangedFields() {}

        /*
		Returns a map containing the values for each field changed with the last server update.
        The 1-based field position within the field schema or field list is used as key for the values in the map.
        Note that if the Subscription mode of the involved Subscription is COMMAND, then changed fields
        are meant as relative to the previous update for the same key. On such tables if a DELETE command
        is received, all the fields, excluding the key field, will be present as changed, with None value.
        All of this is also true on tables that have the two-level behavior enabled, but in case of
        DELETE commands second-level fields will not be iterated.

        :return: A map containing the values for each field changed with the last server update.

        .. seealso:: :meth:`.Subscription.setFieldSchema`
        .. seealso:: :meth:`.Subscription.setFields`
        */
        void getChangedFieldsByPosition() {}

        /*
		Returns a map containing the values for each field in the Subscription.
        The related field name is used as key for the values in the map.

        :raises IllegalStateException: if the Subscription was initialized using a field schema.

        :return: A map containing the values for each field in the Subscription.

        .. seealso:: :meth:`.Subscription.setFieldSchema`
        .. seealso:: :meth:`.Subscription.setFields`
        */
        void getFields() {}

        /*
		Returns a map containing the values for each field in the Subscription.
        The 1-based field position within the field schema or field list is used as key for the values in the map.

        :return: A map containing the values for each field in the Subscription.

        .. seealso:: :meth:`.Subscription.setFieldSchema`
        .. seealso:: :meth:`.Subscription.setFields`
        */
        void getFieldsByPosition() {}

        /*
		Inquiry method that retrieves the name of the item to which this update pertains.

        The name will be None if the related Subscription was initialized using an "Item Group".

        :return: The name of the item to which this update pertains.

        .. seealso:: :meth:`.Subscription.setItemGroup`
        .. seealso:: :meth:`.Subscription.setItems`
        */
        void getItemName() {}

        /*
		Inquiry method that retrieves the position in the "Item List" or "Item Group" of the item to which this update pertains.

        :return: The 1-based position of the item to which this update pertains.

        .. seealso:: :meth:`.Subscription.setItemGroup`
        .. seealso:: :meth:`.Subscription.setItems`
        */
        void getItemPos() {}

        /*
		Inquiry method that gets the value for a specified field, as received from the Server with the current or
        previous update.

        :raises IllegalArgumentException: if the specified field is not part of the Subscription.

        :param fieldNameOrPos: The field name or the 1-based position of the field within the "Field List" or
        "Field Schema".

        :return: The value of the specified field; it can be None in the following cases:

                * a None value has been received from the Server, as None is a possible value for a field;
        * no value has been received for the field yet;
        * the item is subscribed to with the COMMAND mode and a DELETE command is received (only the fields used to
        carry key and command information are valued).

        .. seealso:: :meth:`.Subscription.setFieldSchema`
        .. seealso:: :meth:`.Subscription.setFields`
        */
        void getValue(FieldNameOrPos fieldnameorpos) {}

        /*
		Inquiry method that gets the difference between the new value and the previous one as a JSON Patch structure,
        provided that the Server has used the JSON Patch format to send this difference, as part of the "delta delivery"
        mechanism. This, in turn, requires that:

        * the Data Adapter has explicitly indicated JSON Patch as the privileged type of compression for this field;
        * both the previous and new value are suitable for the JSON Patch computation (i.e. they are valid JSON
        representations);
        * the item was subscribed to in MERGE or DISTINCT mode (note that, in case of two-level behavior, this holds
        for all fields related with second-level items, as these items are in MERGE mode);
        * sending the JSON Patch difference has been evaluated by the Server as more efficient than sending the full
        new value.

        Note that the last condition can be enforced by leveraging the Server's <jsonpatch_min_length> configuration
        flag, so that the availability of the JSON Patch form would only depend on the Client and the Data Adapter.

        When the above conditions are not met, the method just returns None; in this case, the new value can only be
        determined through :meth:`ItemUpdate.getValue`. For instance, this will always be needed to get the first value
         received.

        :raises IllegalArgumentException: if the specified field is not part of the Subscription.

        :param fieldNameOrPos: The field name or the 1-based position of the field within the "Field List" or
        "Field Schema".

        :return: A JSON Patch structure representing the difference between the new value and the previous one, or None
         if the difference in JSON Patch format is not available for any reason.

        .. seealso:: :meth:`ItemUpdate.getValue`
        */
        void getValueAsJSONPatchIfAvailable(FieldNameOrPos fieldnameorpos) {}

        /*
		Inquiry method that asks whether the current update belongs to the item snapshot (which carries the current
        item state at the time of Subscription). Snapshot events are sent only if snapshot information was requested
        for the items through :meth:`.Subscription.setRequestedSnapshot` and precede the real time events. Snapshot
        information take different forms in different subscription modes and can be spanned across zero, one or several
        update events. In particular:

        * if the item is subscribed to with the RAW subscription mode, then no snapshot is sent by the Server;
        * if the item is subscribed to with the MERGE subscription mode, then the snapshot consists of exactly one
        event, carrying the current value for all fields;
        * if the item is subscribed to with the DISTINCT subscription mode, then the snapshot consists of some of the
        most recent updates; these updates are as many as specified through :meth:`.Subscription.setRequestedSnapshot`,
        unless fewer are available;
        * if the item is subscribed to with the COMMAND subscription mode, then the snapshot consists of an "ADD" event
        for each key that is currently present.

        Note that, in case of two-level behavior, snapshot-related updates for both the first-level item (which is in
        COMMAND mode) and any second-level items (which are in MERGE mode) are qualified with this flag.

        :return: true if the current update event belongs to the item snapshot; false otherwise.
        */
        void isSnapshot() {}

        /*
		Inquiry method that asks whether the value for a field has changed after the reception of the last update from
        the Server for an item. If the Subscription mode is COMMAND then the change is meant as relative to the same key.

        :param fieldNameOrPos: The field name or the 1-based position of the field within the field list or field schema.

        :return: Unless the Subscription mode is COMMAND, the return value is true in the following cases:

        * It is the first update for the item;
        * the new field value is different than the previous field value received for the item.

        If the Subscription mode is COMMAND, the return value is true in the following cases:

        * it is the first update for the involved key value (i.e. the event carries an "ADD" command);
        * the new field value is different than the previous field value received for the item, relative to the same
        key value (the event must carry an "UPDATE" command);
        * the event carries a "DELETE" command (this applies to all fields other than the field used to carry key
        information).

        In all other cases, the return value is false.

        :raises IllegalArgumentException: if the specified field is not part of the Subscription.
        */
        void isValueChanged(FieldNameOrPos fieldnameorpos) {}
    };

}