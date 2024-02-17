#ifndef IMCLIENT_H
#define IMCLIENT_H

#include <functional>
#include <wstring.h>
#include <AsyncMqttClient.h>

class IMQTTMediator;

class IMClient
{
public:
    IMClient(){};
   // IMClient(IMQTTMediator *mymediator);*/
    ~IMClient(){};

    virtual void setOnConnect(AsyncMqttClientInternals::OnConnectUserCallback callback)=0;
    virtual void setOnDisconnect(AsyncMqttClientInternals::OnDisconnectUserCallback callback)=0;
    virtual void setOnSubscribe( AsyncMqttClientInternals::OnSubscribeUserCallback callback)=0;
    virtual void setOnUnsubscribe( AsyncMqttClientInternals::OnUnsubscribeUserCallback callback)=0;
    virtual void setOnMessage(AsyncMqttClientInternals::OnMessageUserCallback callback)=0;
    virtual void setOnPublish(AsyncMqttClientInternals::OnPublishUserCallback callback)=0;

    virtual uint16_t subscribe(String topic, uint8_t qos)=0;
    virtual uint16_t unsubscribe(String topic)=0;
    virtual uint16_t publish(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0)=0;
    virtual void setMQTTMediator(IMQTTMediator *mediator)=0;
    virtual bool connected()=0;
};
#endif