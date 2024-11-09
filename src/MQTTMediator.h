#ifndef MQTTMEDIATOR_H
#define MQTTMEDIATOR_H

#include <Arduino.h>
#include <vector>
#include <utility>
#include <functional>
#include <forward_list>
#include <tuple>
#include "IMClient.h"
#include "IMQTTMediator.h"
#include <AsyncMqttClient.h>

//#define DEBUG_MQTTMEDI

#ifdef DEBUG_MQTTMEDI
#define _MQTTMEDI_PP(a) Serial.print(a);
#define _MQTTMEDI_PL(a) Serial.println(a);
#else
#define _MQTTMEDI_PP(a)
#define _MQTTMEDI_PL(a)
#endif


class MQTTMediator: public IMQTTMediator,public AsyncMqttClient
{
 enum ENUM_tuplecontainer
{
    ENUM_MCLIENT_PTR,
    ENUM_TOPIC_V,
    ENUM_PACKETID_V,
    ENUM_USERCBS_ST
};
private:
    std::forward_list<tuplecontainer> _clients;
    uint32_t _lastPacketIdCleaning;
    bool _isClientExist(IMClient* client, const tuplecontainer& it);
    bool _isTopicAdded(String topic_old,String topic_new);
    uint8_t _packetidCleaner(uint32_t cleanolderthan = 30000UL);
    int _mosquitto_topic_matches_sub(const char* sub, const char* topic, bool* result);
    
    std::forward_list<tuplecontainer>::iterator _findClientbyPtr(IMClient* client);
    std::forward_list<tuplecontainer>::iterator _addEmptyClientTuple(IMClient* client);


    //AsyncMqttClientInternals::OnConnectUserCallback onConnectcb=std::bind(_mediatorOnConnect,std::placeholers::_1);
    void _mediatorOnConnect(bool sessionpresent);
    void _mediatorOnDisconnect(AsyncMqttClientDisconnectReason reason);
    void _mediatorOnSubscribe(uint16_t packetId, uint8_t qos);
    void _mediatorOnUnsubscribe(uint16_t packetId);
    void _mediatorOnMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    void _mediatorOnPublish(uint16_t packetId);

    AsyncMqttClientInternals::OnConnectUserCallback     _mediatoronconnectcb=NULL;
    AsyncMqttClientInternals::OnDisconnectUserCallback   _mediatorondisconnectcb=NULL;
    bool _mediatoronconnectexclusively=false;
    bool _mediatorondisconnectexclusively=false;

public:
    MQTTMediator();
    ~MQTTMediator(){};

    void vanishClient(IMClient* client);
    bool connected();
    uint16_t subscribe(IMClient* client, const String topic, uint8_t qos);
    uint16_t unsubscribe(IMClient* client, String topic);
    uint16_t publish(IMClient* client, const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0);

    void setOnConnectClientCB (IMClient* client, AsyncMqttClientInternals::OnConnectUserCallback callback);
    void setOnMediatorConnect(AsyncMqttClientInternals::OnConnectUserCallback callback, bool exclusively); //if you set callback then
    //MQTTMediator get info about MQTTServer connection, if exclusively then the other clients don't.

    void setOnDisconnectClientCB (IMClient* client,  AsyncMqttClientInternals::OnDisconnectUserCallback callback);
    void setOnMediatorDisconnect(AsyncMqttClientInternals::OnDisconnectUserCallback callback, bool exclusively);//if you set callback then
    //MQTTMediator get info about MQTTServer disconnection, if exclusively then the other clients don't.

    void setOnSubscribeClientCB(IMClient* client,  AsyncMqttClientInternals::OnSubscribeUserCallback callback);
    void setOnUnsubscribeClientCB(IMClient* client, AsyncMqttClientInternals::OnUnsubscribeUserCallback callback);
    void setOnMessageClientCB(IMClient* client,  AsyncMqttClientInternals::OnMessageUserCallback callback);
    void setOnPublishClientCB(IMClient* client,  AsyncMqttClientInternals::OnPublishUserCallback callback);
    void setOnErrorClientCB(IMClient* client, AsyncMqttClientInternals::OnErrorUserCallback callback);
};
#endif /* MQTTMEDIATOR_H */
