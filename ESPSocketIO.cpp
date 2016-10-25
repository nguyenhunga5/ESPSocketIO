//
//  ESPSocketIO.cpp
//  firmware
//
//  Created by NguyenHungA5 on 24/10/2016.
//
//

#include "ESPSocketIO.hpp"

ESPSocketIO *_socketIOClient = NULL;

void debugLog(const char *format, ...)
{
    if (_socketIOClient->isDebug)
    {
        char    buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start(args, format);
        vsnprintf(buf, 128, format, args);
        va_end(args);
        Serial.println("[Debug]" + String(buf));
    }
}


void debugLog(String log)
{
    if (_socketIOClient->isDebug)
    {
        Serial.println("[Debug]" + log);
    }
}


void socketIOEvent(WStype_t type, uint8_t *payload, size_t lenght)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        debugLog("[WSc] Disconnected!\r\n");
        _socketIOClient->isConnected = false;
        break;

    case WStype_CONNECTED:
        debugLog("[WSc] Connected to url: %s\r\n", payload);
        _socketIOClient->isConnected = true;

        // send message to server when Connected
        // socket.io upgrade confirmation message (required)
        _socketIOClient->sendText("5");
        break;

    case WStype_TEXT:
        debugLog("[WSc] get text: %s\r\n", payload);
        _socketIOClient->parse(String((char *)payload));
        // send message to server
        // webSocket.sendTXT("message here");
        break;

    case WStype_BIN:
        debugLog("[WSc] get binary lenght: %u\r\n", lenght);
        hexdump(payload, lenght);

        // send data to server
        // webSocket.sendBIN(payload, lenght);
        break;
    }
}


ESPSocketIO::ESPSocketIO()
{
    _socketIOClient          = this;
    this->heartbeatTimestamp = 0;
    this->isConnected        = false;
    this->client.onEvent(socketIOEvent);
}


ESPSocketIO::~ESPSocketIO()
{
    _socketIOClient = NULL;
}


void ESPSocketIO::connect(String host, int port)
{
    this->host = host;
    this->port = port;
    this->client.beginSocketIO(host, port);
}


void ESPSocketIO::disconnect()
{
    this->client.disconnect();
    this->isConnected = false;
}


bool ESPSocketIO::connected()
{
    return this->isConnected;
}


void ESPSocketIO::loop()
{
    this->client.loop();

    unsigned long currentMillis = millis();
    if ((currentMillis - heartbeatTimestamp) > HEARTBEAT_INTERVAL)
    {
        heartbeatTimestamp = currentMillis;
        // socket.io heartbeat message
        this->sendHeartbeat();
    }
}


bool ESPSocketIO::on(String event, SocketIOMessage callback)
{
    this->events.insert(std::pair<String, SocketIOMessage>(event, callback));
}


void ESPSocketIO::emit(String event, String payload)
{
    String message = "42[\"" + event + "\"," + payload + "]";

    debugLog(message);

    this->client.sendTXT(message);
}


void ESPSocketIO::sendText(String text)
{
    debugLog(text);

    this->client.sendTXT(text);
}


void ESPSocketIO::parse(String payload)
{
    debugLog("Parse: " + payload);

    if (payload.length() < 4)
    {
        return;
    }

    int    startIndex = 4;
    int    endIndex   = payload.indexOf("\",");
    String eventName  = payload.substring(startIndex, endIndex);
    payload.remove(0, endIndex + 2);
    // startIndex = payload.indexOf(":\"") + 2;
    // payload.remove(0, startIndex);
    // endIndex = payload.indexOf('\"');
    // String name = payload.substring(0, endIndex);
    // payload.remove(0, endIndex + 2);
    // endIndex = payload.lastIndexOf("}]") + 1;
    // String content = "{" + payload.substring(0, endIndex);

    payload.remove(payload.length() - 1);

    bool isFound = false;
    for (auto const & event : this->events)
    {
        debugLog("Event: " + event.first);
        if (event.first.equals(eventName))
        {
            event.second(payload);
            isFound = true;
            break;
        }
    }

    if (!isFound)
    {
        debugLog("Not found event handler for: " + payload);
    }
}


void ESPSocketIO::sendHeartbeat()
{
    if (this->isConnected)
    {
        this->sendText("2");
    }
}
