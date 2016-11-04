//
//  ESPSocketIO.hpp
//  firmware
//
//  Created by NguyenHungA5 on 24/10/2016.
//
//

#ifndef ESPSocketIO_hpp
#define ESPSocketIO_hpp

#include <stdio.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <functional>
#include <WebSocketsClient.h>

#define HEARTBEAT_INTERVAL    25000

typedef std::function<void (String content)>   SocketIOMessage;

class ESPSocketIO {
private:

    std::map<String, SocketIOMessage> events;
    WebSocketsClient client;
    uint64_t         heartbeatTimestamp;
    void sendHeartbeat();

public:
    ESPSocketIO();
    ~ESPSocketIO();

    bool isDebug;
    bool isConnected;

    String host;
    int    port;

    void connect(String host, int port);
    void disconnect();
    bool connected();

    void loop();

    bool on(String event, SocketIOMessage callback);
    void removeListener(String eventName);
    void emit(String event, String payload);
    void sendText(String text);

    void parse(String payload);

protected:
};


#endif /* ESPSocketIO_hpp */
