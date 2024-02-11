#include "MQTTMediator.h"

/* Does a topic match a subscription? */
// Code from https://github.com/eclipse/mosquitto/blob/master/lib/util_topic.c
// Copyright (c) 2009-2020 Roger Light <roger@atchoo.org>

#define MOSQ_ERR_SUCCESS 0
#define MOSQ_ERR_INVAL 3

int MQTTMediator::_mosquitto_topic_matches_sub(const char *sub, const char *topic, bool *result)
{
    size_t spos;

    if (!result)
        return MOSQ_ERR_INVAL;
    *result = false;

    if (!sub || !topic || sub[0] == 0 || topic[0] == 0)
    {
        return MOSQ_ERR_INVAL;
    }

    if ((sub[0] == '$' && topic[0] != '$') || (topic[0] == '$' && sub[0] != '$'))
    {

        return MOSQ_ERR_SUCCESS;
    }

    spos = 0;

    while (sub[0] != 0)
    {
        if (topic[0] == '+' || topic[0] == '#')
        {
            return MOSQ_ERR_INVAL;
        }
        if (sub[0] != topic[0] || topic[0] == 0)
        { /* Check for wildcard matches */
            if (sub[0] == '+')
            {
                /* Check for bad "+foo" or "a/+foo" subscription */
                if (spos > 0 && sub[-1] != '/')
                {
                    return MOSQ_ERR_INVAL;
                }
                /* Check for bad "foo+" or "foo+/a" subscription */
                if (sub[1] != 0 && sub[1] != '/')
                {
                    return MOSQ_ERR_INVAL;
                }
                spos++;
                sub++;
                while (topic[0] != 0 && topic[0] != '/')
                {
                    if (topic[0] == '+' || topic[0] == '#')
                    {
                        return MOSQ_ERR_INVAL;
                    }
                    topic++;
                }
                if (topic[0] == 0 && sub[0] == 0)
                {
                    *result = true;
                    return MOSQ_ERR_SUCCESS;
                }
            }
            else if (sub[0] == '#')
            {
                /* Check for bad "foo#" subscription */
                if (spos > 0 && sub[-1] != '/')
                {
                    return MOSQ_ERR_INVAL;
                }
                /* Check for # not the final character of the sub, e.g. "#foo" */
                if (sub[1] != 0)
                {
                    return MOSQ_ERR_INVAL;
                }
                else
                {
                    while (topic[0] != 0)
                    {
                        if (topic[0] == '+' || topic[0] == '#')
                        {
                            return MOSQ_ERR_INVAL;
                        }
                        topic++;
                    }
                    *result = true;
                    return MOSQ_ERR_SUCCESS;
                }
            }
            else
            {
                /* Check for e.g. foo/bar matching foo/+/# */
                if (topic[0] == 0 && spos > 0 && sub[-1] == '+' && sub[0] == '/' && sub[1] == '#')
                {
                    *result = true;
                    return MOSQ_ERR_SUCCESS;
                }

                /* There is no match at this point, but is the sub invalid? */
                while (sub[0] != 0)
                {
                    if (sub[0] == '#' && sub[1] != 0)
                    {
                        return MOSQ_ERR_INVAL;
                    }
                    spos++;
                    sub++;
                }

                /* Valid input, but no match */
                return MOSQ_ERR_SUCCESS;
            }
        }
        else
        {
            /* sub[spos] == topic[tpos] */
            if (topic[1] == 0)
            {
                /* Check for e.g. foo matching foo/# */
                if (sub[1] == '/' && sub[2] == '#' && sub[3] == 0)
                {
                    *result = true;
                    return MOSQ_ERR_SUCCESS;
                }
            }
            spos++;
            sub++;
            topic++;
            if (sub[0] == 0 && topic[0] == 0)
            {
                *result = true;
                return MOSQ_ERR_SUCCESS;
            }
            else if (topic[0] == 0 && sub[0] == '+' && sub[1] == 0)
            {
                if (spos > 0 && sub[-1] != '/')
                {
                    return MOSQ_ERR_INVAL;
                }
                spos++;
                sub++;
                *result = true;
                return MOSQ_ERR_SUCCESS;
            }
        }
    }
    if ((topic[0] != 0 || sub[0] != 0))
    {
        *result = false;
    }
    while (topic[0] != 0)
    {
        if (topic[0] == '+' || topic[0] == '#')
        {
            return MOSQ_ERR_INVAL;
        }
        topic++;
    }

    return MOSQ_ERR_SUCCESS;
}

bool MQTTMediator::_isClientExist(IMClient *client, const tuplecontainer &it)
{
    if (std::get<ENUM_MCLIENT_PTR>(it) == client)
        return true;
    return false;
}

bool MQTTMediator::_isTopicAdded(String topic_existing, String topic_new)
{
    if (topic_existing == topic_new)
        return true;
    return false;
}

std::forward_list<tuplecontainer>::iterator MQTTMediator::_findClientbyPtr(IMClient *client)
{
    if (client == nullptr)
    {
        return _clients.end();
    }
    std::forward_list<tuplecontainer>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); it++)
    {
        if (_isClientExist(client, *it))
        {
            break;
        }
    }
    return it;
}

std::forward_list<tuplecontainer>::iterator MQTTMediator::_addEmptyClientTuple(IMClient* client)
{
    _clients.emplace_front(std::make_tuple(client, std::vector<String>{}, std::vector<std::pair<uint16_t, uint32_t>>(), UserCBs{}));
    return  _clients.begin();
}

uint8_t MQTTMediator::_packetidCleaner(uint32_t cleanolderthan)
{
    if (millis() - _lastPacketIdCleaning < PACKETCLEANINGPERIOD)
        return 0;
    _lastPacketIdCleaning = millis();
    uint16_t count = 0;
    for (auto it = _clients.begin(); it != _clients.end(); it++)
    {
        for (auto it2 = std::get<ENUM_PACKETID_V>(*it).begin(); it2 != std::get<ENUM_PACKETID_V>(*it).end(); it2++)
        {
            if (millis() - it2->second >= cleanolderthan)
            {
                std::get<ENUM_PACKETID_V>(*it).erase(it2);
                it2--;
                count++;
            }
        }
    }
    _MQTTMEDI_PL(String(__FUNCTION__)+String(count) + String(" packetID cleared."));
    return count;
}

uint16_t MQTTMediator::publish(IMClient *client, const char *topic, uint8_t qos, bool retain, const char *payload, size_t length, bool dup, uint16_t message_id)
{
    _packetidCleaner();
    if (client == nullptr)
        return 0;
    auto it = _findClientbyPtr(client);
    if (it == _clients.end())
    {
        it = _addEmptyClientTuple(client);
    }
    uint16_t paketid = AsyncMqttClient::publish(topic, qos, retain, payload, length, dup, message_id);
    if (paketid && qos)
        std::get<ENUM_PACKETID_V>(*it).emplace_back(std::make_pair(paketid, millis()));
    return paketid;
}

MQTTMediator::MQTTMediator()
{
    // AsyncMqttClient::onConnect(std::bind(&MQTTMediator::_mediatorOnConnect, this, std::placeholders::_1));
    AsyncMqttClient::onConnect([&, this](bool sessionpresent) -> void
                               { this->_mediatorOnConnect(sessionpresent); });
    AsyncMqttClient::onDisconnect([&, this](AsyncMqttClientDisconnectReason reason) -> void
                                  { this->_mediatorOnDisconnect(reason); });
    AsyncMqttClient::onSubscribe([&, this](uint16_t packetId, uint8_t qos) -> void
                                 { this->_mediatorOnSubscribe(packetId, qos); });
    AsyncMqttClient::onUnsubscribe([&, this](uint16_t packetId) -> void
                                   { _mediatorOnUnsubscribe(packetId); });
    AsyncMqttClient::onMessage([&, this](char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
                               { this->_mediatorOnMessage(topic, payload, properties, len, index, total); });
    AsyncMqttClient::onPublish([&, this](uint16_t packetId) -> void
                               { this->_mediatorOnPublish(packetId); });

    _lastPacketIdCleaning = millis();
}

void MQTTMediator::_mediatorOnConnect(bool sessionpresent)
{
    bool sp=sessionpresent;
    if (_bootednow)sp=false;//force resubscribe clients
    if (_mediatoronconnectcb)
    {
        std::invoke(_mediatoronconnectcb, sp);
    }
    if(!_mediatoronconnectexclusively)
    {
        for (auto it = _clients.begin(); it != _clients.end(); it++)
        {
            if (std::get<ENUM_USERCBS_ST>(*it).onConnectucb)
            {
                std::invoke(std::get<ENUM_USERCBS_ST>(*it).onConnectucb, sp);
            }
        }
    }
    _bootednow=false;
}

void MQTTMediator::_mediatorOnDisconnect(AsyncMqttClientDisconnectReason reason)
{
    if (_mediatorondisconnectcb)
    {
        std::invoke(_mediatorondisconnectcb, reason);
    }
    if(!_mediatorondisconnectexclusively)
    {
        for (auto it = _clients.begin(); it != _clients.end(); it++)
        {
            if (std::get<ENUM_USERCBS_ST>(*it).onDisconnectucb)
                std::invoke(std::get<ENUM_USERCBS_ST>(*it).onDisconnectucb, reason);
        }
    }
}

void MQTTMediator::_mediatorOnSubscribe(uint16_t packetId, uint8_t qos)
{
    for (auto it = _clients.begin(); it != _clients.end(); it++)
    {
        for (auto it2 = std::get<ENUM_PACKETID_V>(*it).begin(); it2 != std::get<ENUM_PACKETID_V>(*it).end(); it2++)
        {
            if (it2->first == packetId)
            {   
                if (std::get<ENUM_USERCBS_ST>(*it).onSubscribeucb)
                {
                    _MQTTMEDI_PL(String(__FUNCTION__)+":packetid match: " + String(packetId));
                    std::invoke(std::get<ENUM_USERCBS_ST>(*it).onSubscribeucb, packetId, qos);
                    std::get<ENUM_PACKETID_V>(*it).erase(it2);
                    return;
                }
            }
        }
    }
}

void MQTTMediator::_mediatorOnUnsubscribe(uint16_t packetId)
{
    for (auto it = _clients.begin(); it != _clients.end(); it++)
    {
        for (auto it2 = std::get<ENUM_PACKETID_V>(*it).begin(); it2 != std::get<ENUM_PACKETID_V>(*it).end(); it2++)
        {
            if (it2->first == packetId)
            {
                if (std::get<ENUM_USERCBS_ST>(*it).onUnsubscribeucb)
                {
                    std::invoke(std::get<ENUM_USERCBS_ST>(*it).onUnsubscribeucb, packetId);
                    std::get<ENUM_PACKETID_V>(*it).erase(it2);
                    return;
                }
            }
        }
    }
}

void MQTTMediator::_mediatorOnMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    for (auto it = _clients.begin(); it != _clients.end(); it++)
    {
        bool match;
        for (auto it2 = std::get<ENUM_TOPIC_V>(*it).begin(); it2 != std::get<ENUM_TOPIC_V>(*it).end(); it2++)
        {
            if (_mosquitto_topic_matches_sub((*it2).c_str(), topic, &match) == MOSQ_ERR_SUCCESS)
            {
                if (match)
                {
                    if (std::get<ENUM_USERCBS_ST>(*it).onMessageucb)
                        std::invoke(std::get<ENUM_USERCBS_ST>(*it).onMessageucb, topic, payload, properties, len, index, total);
                }
            }
            else
            {
                _MQTTMEDI_PL(String(__FUNCTION__)+":Invalid subscripton or topic");
            }
        }
    }
    _packetidCleaner();
}

void MQTTMediator::_mediatorOnPublish(uint16_t packetId)
{
    _packetidCleaner();
    for (auto it = _clients.begin(); it != _clients.end(); it++)
    {
        for (auto it2 = std::get<ENUM_PACKETID_V>(*it).begin(); it2 != std::get<ENUM_PACKETID_V>(*it).end(); it2++)
        {
            if (it2->first == packetId)
            {
                if (std::get<ENUM_USERCBS_ST>(*it).onPublishucb)
                {
                    std::invoke(std::get<ENUM_USERCBS_ST>(*it).onPublishucb, packetId);
                    std::get<ENUM_PACKETID_V>(*it).erase(it2);
                    return;
                }
            }
        }
    }
}

void MQTTMediator::setOnConnectClientCB(IMClient *client, AsyncMqttClientInternals::OnConnectUserCallback callback)
{
    if (client == nullptr)
        return;
    auto it = _findClientbyPtr(client);
    if (it == _clients.end())
    {
        it = _addEmptyClientTuple(client);
    }
    std::get<ENUM_USERCBS_ST>(*it).onConnectucb = callback;
}

void MQTTMediator::setOnDisconnectClientCB(IMClient *client, AsyncMqttClientInternals::OnDisconnectUserCallback callback)
{
    if (client == nullptr)
        return;
    auto it = _findClientbyPtr(client);
    if (it == _clients.end())
    {
        it = _addEmptyClientTuple(client);
    }
    std::get<ENUM_USERCBS_ST>(*it).onDisconnectucb = callback;
}

void MQTTMediator::setOnSubscribeClientCB(IMClient *client, AsyncMqttClientInternals::OnSubscribeUserCallback callback)
{
    if (client == nullptr)
        return;
    auto it = _findClientbyPtr(client);
    if (it == _clients.end())
    {
        it = _addEmptyClientTuple(client);
    }
    std::get<ENUM_USERCBS_ST>(*it).onSubscribeucb = callback;
}
void MQTTMediator::setOnUnsubscribeClientCB(IMClient *client, AsyncMqttClientInternals::OnUnsubscribeUserCallback callback)
{
    if (client == nullptr)
        return;
    auto it = _findClientbyPtr(client);
    if (it == _clients.end())
    {
        it = _addEmptyClientTuple(client);
    }
    std::get<ENUM_USERCBS_ST>(*it).onUnsubscribeucb = callback;
}

void MQTTMediator::setOnMessageClientCB(IMClient *client, AsyncMqttClientInternals::OnMessageUserCallback callback)
{
    if (client == nullptr)
        return;
    auto it = _findClientbyPtr(client);
    if (it == _clients.end())
    {
        it = _addEmptyClientTuple(client);
    }
    std::get<ENUM_USERCBS_ST>(*it).onMessageucb = callback;
}
void MQTTMediator::setOnPublishClientCB(IMClient *client, AsyncMqttClientInternals::OnPublishUserCallback callback)
{
    if (client == nullptr)
        return;
    auto it = _findClientbyPtr(client);
    if (it == _clients.end())
    {
        it = _addEmptyClientTuple(client);
    }
    std::get<ENUM_USERCBS_ST>(*it).onPublishucb = callback;
}

void MQTTMediator::setOnErrorClientCB(IMClient *client, AsyncMqttClientInternals::OnErrorUserCallback callback)
{
    if (client == nullptr)
        return;
    auto it = _findClientbyPtr(client);
    if (it == _clients.end())
    {
        it = _addEmptyClientTuple(client);
    }
    std::get<ENUM_USERCBS_ST>(*it).onErrorucb = callback;
}

void MQTTMediator::vanishClient(IMClient *client)
{
    if (client == nullptr)
        return;

    auto prev = _clients.before_begin();
    for (auto it = _clients.begin(); it != _clients.end(); prev = it++)
    {
        if (_isClientExist(client, *it))
        {
            _clients.erase_after(prev);
            break;
        }
    }
}

uint16_t MQTTMediator::subscribe(IMClient *client, const String topic, uint8_t qos)
{
    if (topic == "" || client == nullptr)
        return 0;

    std::forward_list<tuplecontainer>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); it++)
    {
        if (_isClientExist(client, *it))
        {
            break;
        }
    }

    if (it == _clients.end())
    {
        _clients.emplace_front(std::make_tuple(client, std::vector<String>{topic}, std::vector<std::pair<uint16_t, uint32_t>>(), UserCBs{}));
        it = _clients.begin();
    }
    else
    {
        auto it2 = std::find_if(std::get<ENUM_TOPIC_V>(*it).begin(), std::get<ENUM_TOPIC_V>(*it).end(),
                                [&, this](String existing_topic)
                                { return this->_isTopicAdded(existing_topic, topic); });

        if (it2 == std::get<ENUM_TOPIC_V>(*it).end())
        std::get<ENUM_TOPIC_V>(*it).emplace_back(topic);
    }
    uint16_t packet_id = AsyncMqttClient::subscribe(topic.c_str(), qos); //subscribe or resubcribe with qos
    if (packet_id)
        std::get<ENUM_PACKETID_V>(*it).emplace_back(std::make_pair(packet_id, millis()));
    return packet_id;
}

uint16_t MQTTMediator::unsubscribe(IMClient *client, String topic)
{
    if (topic == "" || client == nullptr)
        return 0;
    uint16_t packetid = 0;
    std::forward_list<tuplecontainer>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); it++)
    {
        if (_isClientExist(client, *it))
        {
            break;
        }
    }
    if (it == _clients.end())
    {
        return 0; // Client not found
    }

    for (auto it2 = std::get<ENUM_TOPIC_V>(*it).begin(); it2 != std::get<ENUM_TOPIC_V>(*it).end(); it2++)
    {
        if (*it2 == topic)
        {
            std::get<ENUM_TOPIC_V>(*it).erase(it2);
            packetid = AsyncMqttClient::unsubscribe(topic.c_str());
            if(packetid)
            {
            std::get<ENUM_PACKETID_V>(*it).emplace_back(std::make_pair(packetid, millis()));
            }
            break;
        }
    }
    return packetid;
}
bool MQTTMediator::connected()
{
    return AsyncMqttClient::connected();
}
void MQTTMediator::setOnMediatorConnect(AsyncMqttClientInternals::OnConnectUserCallback callback, bool exlusively)
{
    _mediatoronconnectcb = callback;
    _mediatoronconnectexclusively = exlusively;
}
void MQTTMediator::setOnMediatorDisconnect(AsyncMqttClientInternals::OnDisconnectUserCallback callback, bool exlusively)
{
    _mediatorondisconnectcb = callback;
    _mediatorondisconnectexclusively =exlusively;
}