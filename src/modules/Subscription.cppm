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

    class Subscription {
    public:

        void addListener(Listener listener) {}

        void getCommandPosition() {}

        void getCommandSecondLevelDataAdapter() {}

        void getCommandSecondLevelFields() {}

        void getCommandSecondLevelFieldSchema() {}

        void getCommandValue(ItemNameOrPos itemnameorpos, KeyValue keyvalue, FieldNameOrPos fieldnameorpos) {}

        void getDataAdapter() {}

        void getFields() {}

        void getFieldSchema() {}

        void getItemGroup() {}

        void getItems() {}

        void getKeyPosition() {}

        void getListeners() {}

        void getMode() {}

        void getRequestedBufferSize() {}

        void getRequestedMaxFrequency() {}

        void getRequestedSnapshot() {}

        void getSelector() {}

        void getValue(ItemNameOrPos itemnameorpos, FieldNameOrPos fieldnameorpos) {}

        void isActive() {}

        void isSubscribed() {}

        void removeListener(Listener listener) {}

        void setCommandSecondLevelDataAdapter(DataAdapter dataadapter) {}

        void setCommandSecondLevelFields(Fields fields) {}

        void setCommandSecondLevelFieldSchema(Schema schema) {}

        void setDataAdapter(DataAdapter dataadapter) {}

        void setFields(Fields fields) {}

        void setFieldSchema(Schema schema) {}

        void setItemGroup(Group group) {}

        void setItems(Items items) {}

        void setRequestedBufferSize(Size size) {}

        void setRequestedMaxFrequency(Frequency frequency) {}

        void setRequestedSnapshot(Snapshot snapshot) {}

        void setSelector(Selector selector) {}


    };
}