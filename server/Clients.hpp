#pragma once
#include "external/nbnet.hpp"
#include <array>
#include <vector>
#include <iostream>
template <int numClients>
class Clients{
public:
    bool isFull(){return m_activeConnections>=connections.size();}
    void addClient(NBN_ConnectionHandle connection);
    void removeClient(const NBN_ConnectionHandle connection);
    int getActiveConnectionCount(){return m_activeConnections;}
    void shareMessageToOthers(const NBN_MessageInfo& message);
private:
    std::array<NBN_ConnectionHandle, numClients> connections{};
    int m_activeConnections{0};
};

template<int numClients>
void Clients<numClients>::addClient(NBN_ConnectionHandle connection)
{
    assert(m_activeConnections<connections.size());
    bool added{false};
    for(auto& p : connections){
        if(p) continue;

        p=connection;
        m_activeConnections++;
        added=true;
        break;
    }
    assert(added);
}

template<int numClients>
void Clients<numClients>::removeClient(const NBN_ConnectionHandle connection)
{
    bool removed{false};
    for(auto& p : connections){
        if(!p||p!=connection)continue;

        p=0;
        removed=true;
        m_activeConnections--;
        break;
    }
    assert(removed);
}

template<int numClients>
void Clients<numClients>::shareMessageToOthers(const NBN_MessageInfo& message)
{
    assert(message.type == NBN_BYTE_ARRAY_MESSAGE_TYPE);
    NBN_ByteArrayMessage *dataMessage = (NBN_ByteArrayMessage*)message.data;

    std::vector<uint8_t> data;
    data.reserve(dataMessage->length+1);
    data.push_back(message.sender);
    data.insert(data.end(),dataMessage->bytes,dataMessage->bytes+dataMessage->length);
    NBN_ByteArrayMessage_Destroy(dataMessage);

    for(auto& p : connections){
        if(!p||p==message.sender)continue;

        if(NBN_GameServer_SendUnreliableByteArrayTo(p, data.data(), data.size())<0){
            NBN_LogError("Failed to send message.");
        }
    }
}
