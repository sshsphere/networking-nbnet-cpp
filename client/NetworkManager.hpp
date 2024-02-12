#pragma once
#include <string_view>
#include <functional>

struct Message;

class NetworkManager{
public:
    enum MessageType{
        ECHO,
        PLAYER_DATA,
        NUM_MESSAGE_TYPES
    };
    NetworkManager();
    ~NetworkManager();
    void pollEvents(std::function<void(const Message&)> messageCallback);
    void sendEcho(std::string_view msg);
    void sendBytes(MessageType type, uint8_t* data, uint32_t length);
    void sendStruct(MessageType type, const auto& p);
    bool isConnected() {return m_isConnected;}
private:
    void onConnected();
    void onDisconnected();
    void onMessageReceived(std::function<void(const Message&)> messageCallback);
    void sendVector(std::vector<uint8_t>& vec);
    void sendVectorReliable(std::vector<uint8_t>& vec);
    bool m_isConnected{false};
    int m_busyCode{};
};
struct Message{
    NetworkManager::MessageType type{NetworkManager::NUM_MESSAGE_TYPES};
    uint8_t clientId{255};
    uint8_t* data{nullptr};
    uint32_t length{0};
};
void NetworkManager::sendStruct(MessageType type, const auto& p)
{
    size_t structSize = sizeof(p);
    const uint8_t* rawData=reinterpret_cast<const uint8_t*>(&p);
    std::vector<uint8_t> data;
    data.reserve(structSize+1);
    data.push_back(type);
    data.insert(data.end(),rawData, rawData+structSize);

    sendVector(data);
}