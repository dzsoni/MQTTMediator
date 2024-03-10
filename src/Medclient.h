#ifndef MedCLIENT_H
#define MedCLIENT_H

#include <functional>
#include <wstring.h>
#include <AsyncMqttClient.h>
#include "IMClient.h"
#include "IMQTTMediator.h"


class MedClient:public IMClient
{
private:
     IMQTTMediator *_mymediator;
     /*String _ID;
     String _deviceType;*/
     

public:
    MedClient();
    MedClient(IMQTTMediator *mymediator);
    ~MedClient();

    void setOnConnect(AsyncMqttClientInternals::OnConnectUserCallback callback);
    void setOnDisconnect(AsyncMqttClientInternals::OnDisconnectUserCallback callback);
    void setOnSubscribe( AsyncMqttClientInternals::OnSubscribeUserCallback callback);
    void setOnUnsubscribe( AsyncMqttClientInternals::OnUnsubscribeUserCallback callback);
    void setOnMessage(AsyncMqttClientInternals::OnMessageUserCallback callback);
    void setOnPublish(AsyncMqttClientInternals::OnPublishUserCallback callback);

    uint16_t subscribe(String topic, uint8_t qos);
    uint16_t unsubscribe(String topic);
    uint16_t publish(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0);
    void setMQTTMediator(IMQTTMediator *mediator);
    bool connected(){return _mymediator->connected();};

};
#endif