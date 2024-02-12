#include "NetworkManager.hpp"
#include "external/nbnet.hpp"

#include <iostream>
#include <vector>

NetworkManager::NetworkManager()
{
#ifdef __EMSCRIPTEN__
    NBN_WebRTC_Register((NBN_WebRTC_Config){.enable_tls = false}); // Register the WebRTC driver
#else
    NBN_UDP_Register(); // Register the UDP driver
#endif // __EMSCRIPTEN__
    // Initialize the client with a protocol name (must be the same than the one used by the server), the server ip address and port
    if (NBN_GameClient_Start("nbnet-example", "127.0.0.1", 42042) < 0)
    {
        NBN_LogError("Failed to start client");
    }
    m_busyCode=42;
}

NetworkManager::~NetworkManager()
{
    NBN_GameClient_Stop();
    NBN_LogInfo("Disconnected");
}

void NetworkManager::pollEvents(std::function<void(const Message&)> messageCallback)
{
    int ev;

    // Poll for client events
    while ((ev = NBN_GameClient_Poll()) != NBN_NO_EVENT)
    {
        if (ev < 0)
        {
            NBN_LogError("An error occured while polling client events.");
            break;
        }

        switch (ev)
        {
            // Client is connected to the server
            case NBN_CONNECTED:
                onConnected();
                break;

                // Client has disconnected from the server
            case NBN_DISCONNECTED:
                onDisconnected();
                break;

                // A message has been received from the server
            case NBN_MESSAGE_RECEIVED:
                onMessageReceived(messageCallback);
                break;
        }
    }

    // Pack all enqueued messages as packets and send them
    if (NBN_GameClient_SendPackets() < 0)
    {
        NBN_LogError("Failed to send packets.");
    }
}

void NetworkManager::sendEcho(std::string_view msg)
{   
    std::vector<uint8_t> data;
    data.reserve(msg.size()+1);
    data.push_back(NetworkManager::ECHO);
    data.insert(data.end(),msg.begin(), msg.end());

    sendVectorReliable(data);
}

void NetworkManager::sendBytes(MessageType type, uint8_t* data, uint32_t length)
{
    std::vector<uint8_t> dataVector;
    dataVector.reserve(length+1);
    dataVector.push_back(type);
    dataVector.insert(dataVector.end(),data, data+length);

    sendVector(dataVector);
}

void NetworkManager::onConnected(){
    NBN_LogInfo("Connected");
    m_isConnected=true;
}

void NetworkManager::onDisconnected()
{
    NBN_LogInfo("Disconnected");

    m_isConnected = false;

    if (NBN_GameClient_GetServerCloseCode() == m_busyCode)
    {
        NBN_LogInfo("Another client is already connected");
    }
}

void NetworkManager::onMessageReceived(std::function<void(const Message&)> messageCallback)
{
    Message message;
    // Get info about the received message
    NBN_MessageInfo msg_info = NBN_GameClient_GetMessageInfo();

    assert(msg_info.type == NBN_BYTE_ARRAY_MESSAGE_TYPE);

    // Retrieve the received message
    NBN_ByteArrayMessage* msg{(NBN_ByteArrayMessage *)msg_info.data};
    
    message.clientId=msg->bytes[0];
    message.type=static_cast<MessageType>(msg->bytes[1]);
    message.data=msg->bytes+2;
    message.length=msg->length-2;

    messageCallback(message);

    // Destroy the received message
    NBN_ByteArrayMessage_Destroy(msg);
}

void NetworkManager::sendVector(std::vector<uint8_t>& vec)
{
    if(NBN_GameClient_SendUnreliableByteArray(vec.data(), vec.size())<0){
        NBN_LogError("Failed to send data.");
    }
}
void NetworkManager::sendVectorReliable(std::vector<uint8_t>& vec)
{
    if(NBN_GameClient_SendReliableByteArray(vec.data(), vec.size())<0){
        NBN_LogError("Failed to send data.");
    }
}