#ifndef IMQTTMEDIATOR_H
#define IMQTTMEDIATOR_H

#include <Arduino.h>
#include <vector>
#include <utility>
#include <functional>
#include <forward_list>
#include <tuple>
#include <AsyncMqttClient.h>


class IMClient;

struct UserCBs
{
    AsyncMqttClientInternals::OnConnectUserCallback onConnectucb;
    AsyncMqttClientInternals::OnDisconnectUserCallback onDisconnectucb;
    AsyncMqttClientInternals::OnSubscribeUserCallback onSubscribeucb;
    AsyncMqttClientInternals::OnUnsubscribeUserCallback onUnsubscribeucb;
    AsyncMqttClientInternals::OnMessageUserCallback onMessageucb;
    AsyncMqttClientInternals::OnPublishUserCallback onPublishucb;
    AsyncMqttClientInternals::OnErrorUserCallback onErrorucb;
};

#define PACKETCLEANINGPERIOD 30000UL
 typedef   std::tuple<IMClient*,std::vector<String>,std::vector<std::pair<uint16_t,uint32_t>>,UserCBs> tuplecontainer;


class IMQTTMediator
{

private:
    
    virtual bool _isClientExist(IMClient* client, const tuplecontainer& it)=0;
    virtual bool _isTopicAdded(String topic_old,String topic_new)=0;
    virtual uint8_t _packetidCleaner(uint32_t cleanolderthan = 5000UL)=0;
    virtual int _mosquitto_topic_matches_sub(const char* sub, const char* topic, bool* result)=0;
    virtual std::forward_list<tuplecontainer>::iterator _findClientbyPtr(IMClient* client)=0;
    virtual std::forward_list<tuplecontainer>::iterator _addEmptyClientTuple(IMClient* client)=0;


    //AsyncMqttClientInternals::OnConnectUserCallback onConnectcb=std::bind(_mediatorOnConnect,std::placeholers::_1);
    virtual void _mediatorOnConnect(bool sessionpresent)=0;
    virtual void _mediatorOnDisconnect(AsyncMqttClientDisconnectReason reason)=0;
    virtual void _mediatorOnSubscribe(uint16_t packetId, uint8_t qos)=0;
    virtual void _mediatorOnUnsubscribe(uint16_t packetId)=0;
    virtual void _mediatorOnMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)=0;
    virtual void _mediatorOnPublish(uint16_t packetId)=0;

public:
    IMQTTMediator(){};
    virtual ~IMQTTMediator(){};

    virtual void vanishClient(IMClient* client)=0;
    virtual bool connected()=0;
    virtual uint16_t subscribe(IMClient* client, const String topic, uint8_t qos)=0;
    virtual uint16_t unsubscribe(IMClient* client, String topic)=0;
    virtual uint16_t publish(IMClient* client, const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0)=0;

    virtual void setOnConnectClientCB (IMClient* client, AsyncMqttClientInternals::OnConnectUserCallback callback)=0;
    virtual void setOnDisconnectClientCB (IMClient* client,  AsyncMqttClientInternals::OnDisconnectUserCallback callback)=0;
    virtual void setOnSubscribeClientCB(IMClient* client,  AsyncMqttClientInternals::OnSubscribeUserCallback callback)=0;
    virtual void setOnUnsubscribeClientCB(IMClient* client, AsyncMqttClientInternals::OnUnsubscribeUserCallback callback)=0;
    virtual void setOnMessageClientCB(IMClient* client,  AsyncMqttClientInternals::OnMessageUserCallback callback)=0;
    virtual void setOnPublishClientCB(IMClient* client,  AsyncMqttClientInternals::OnPublishUserCallback callback)=0;
    virtual void setOnErrorClientCB(IMClient* client, AsyncMqttClientInternals::OnErrorUserCallback callback)=0;
};
#endif