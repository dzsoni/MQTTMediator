#include "Medclient.h"

MedClient::MedClient()
{
}

MedClient::MedClient(IMQTTMediator *mymediator)
{
    if(mymediator!=nullptr)_mymediator=mymediator;
}

MedClient::~MedClient()
{
    if(_mymediator!=nullptr)
    {
    _mymediator->vanishClient(this);
    }
}

uint16_t MedClient::subscribe(String topic, uint8_t qos)
{
    if(_mymediator == nullptr)return 0;
    return _mymediator->subscribe(this,topic, qos);
}

uint16_t MedClient::unsubscribe(String topic)
{
    if(_mymediator == nullptr)return 0;
    return _mymediator->unsubscribe(this, topic);
}

 uint16_t MedClient::publish(const char* topic, uint8_t qos, bool retain, const char* payload, size_t length , bool dup, uint16_t message_id)
 {
     if(_mymediator!=nullptr)
     {
       return _mymediator->publish(this,topic,qos,retain,payload,length,dup,message_id);
     }
     return 0;
 }

 void MedClient::setMQTTMediator(IMQTTMediator *mediator)
 {
  if(mediator!=nullptr) _mymediator=mediator;
 }



void MedClient::setOnConnect(AsyncMqttClientInternals::OnConnectUserCallback callback) {
 if(_mymediator!=nullptr)
 {
    _mymediator->setOnConnectClientCB(this,callback);
 }
}

void MedClient::setOnDisconnect(AsyncMqttClientInternals::OnDisconnectUserCallback callback) {
  if(_mymediator!=nullptr)
 {
   _mymediator->setOnDisconnectClientCB(this,callback); 
 }
}

void MedClient::setOnSubscribe( AsyncMqttClientInternals::OnSubscribeUserCallback callback) {
  if(_mymediator!=nullptr)
 {
    _mymediator->setOnSubscribeClientCB(this,callback);
 }
}

void MedClient::setOnUnsubscribe( AsyncMqttClientInternals::OnUnsubscribeUserCallback callback) {
  if(_mymediator!=nullptr)
 {
    _mymediator->setOnUnsubscribeClientCB(this,callback);
 }
}

void MedClient::setOnMessage(AsyncMqttClientInternals::OnMessageUserCallback callback) {
 if(_mymediator!=nullptr)
 {
    _mymediator->setOnMessageClientCB(this,callback);
 }
}

void MedClient::setOnPublish( AsyncMqttClientInternals::OnPublishUserCallback callback) {
  if(_mymediator!=nullptr)
 {
    _mymediator->setOnPublishClientCB(this,callback);
 }
}