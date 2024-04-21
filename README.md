# Lightstreamer C++ Client SDK

This project is an open-source client library that enables any application that supports C++ to communicate bidirectionally with a **Lightstreamer Server**.

It is a porting of the [Lightstreamer .Net Client SDK](https://github.com/Lightstreamer/Lightstreamer-lib-client-dotnet) to the C++ language.

## What is Lightstreamer?

Lightstreamer C++ Client SDK enables any application that supports C++ to communicate bidirectionally with a **Lightstreamer Server**.
The API allows you to subscribe to real-time data pushed by a Lightstreamer server and to send any messages to the server.

The library offers automatic recovery from connection failures, automatic selection of the best available transport, and full decoupling of subscription and connection operations.
It is responsible for forwarding the subscriptions to the server and re-forwarding all the subscriptions whenever the connection is broken and then reopened.

## Quickstart

To connect to a Lightstreamer Server, you need to create and configure a `LightstreamerClient` object and instruct it to connect to a specified endpoint.
A minimal version of the code that creates a `LightstreamerClient` and connects to the Lightstreamer Server on *https://push.lightstreamer.com* might look like this:

```cpp
LightstreamerClient client("https://push.lightstreamer.com/", "DEMO");
client.connect();
```

To receive real-time updates from the Lightstreamer server, the client needs to subscribe to specific items handled by a Data Adapter deployed at the server-side.
This can be accomplished by instantiating an object of type Subscription.
For more details about Subscription in Lightstreamer, see section 3.2 of the Lightstreamer General Concepts documentation.
A sample code that subscribes to three items from the classic Stock-List example is:

```cpp
Subscription s_stocks("MERGE");

s_stocks.setFields({"last_price", "time", "stock_name"});
s_stocks.setItems({"item1", "item2", "item16"});
s_stocks.setDataAdapter("QUOTE_ADAPTER");
s_stocks.setRequestedMaxFrequency("3.0");

s_stocks.addListener(std::make_shared<QuoteListener>());

client.subscribe(s_stocks);

```

Before sending the subscription to the server, usually at least one SubscriptionListener is attached to the Subscription instance in order to consume the real-time updates.
The following code demonstrates printing the value of changed fields each time a new update is received for the subscription:

```cpp
class QuoteListener : public SubscriptionListener {
public:
    void onClearSnapshot(const std::string& itemName, int itemPos) override {
        std::cout << "Clear Snapshot for " << itemName << "." << std::endl;
    }

    void onItemUpdate(ItemUpdate itemUpdate) override {
        std::cout << "New update for " << itemUpdate.getItemName() << std::endl;
        for (const auto& field : itemUpdate.getChangedFields()) {
            std::cout << " >>>>>>>>>>>>> " << field.second << std::endl;
        }
    }

    // Additional listener methods are implemented similarly
};

```

## Building ##

To build the library, follow these steps:

Create a new project in your preferred C++ IDE (e.g., Visual Studio for C++).
Add all existing source files from the Lightstreamer_CPP_Client directory.
Include necessary third-party libraries for networking and threading.

## Compatibility ##

The library requires Lightstremaer Server 7.0.1 or later

## Documentation

No specific documentation is available for this library, but you can refer to the [Lightstreamer C++ Client API Reference](https://lightstreamer.com/api/ls-cpp-client/latest/index.html) for a complete reference of the library.

## Other GitHub projects using this library

TODO

## Support

For questions and support please use the [Joaquin Bejar](jb@taunais.com). The issue list of this page is **exclusively** for bug reports and feature requests.

## License

[GNU 3](https://opensource.org/license/gpl-3-0)
