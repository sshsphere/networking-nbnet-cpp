#include "external/nbnet.hpp"

#ifdef USE_WEBRTC_DRIVER
extern "C"{
#include "external/net_drivers/webrtc_c.h"
}
#endif

#include "Clients.hpp"
#include <chrono>
#include <iostream>
#include <array>
#include <thread>
using namespace std::literals::chrono_literals;

int main() {
    SetTraceLogLevel(LOG_DEBUG);
    bool error{false};
    int m_busyCode{42};
    Clients<16> clients;

    NBN_UDP_Register(); 
    #ifdef USE_WEBRTC_DRIVER
    const char *ice_servers[] = { "stun:stun01.sipphone.com" };
    NBN_WebRTC_C_Config cfg = {
        .enable_tls = false,
        .cert_path = NULL,
        .key_path = NULL,
        .passphrase = NULL,
        .ice_servers = ice_servers,
        .ice_servers_count = 1,
        .log_level = RTC_LOG_VERBOSE
    };
    NBN_WebRTC_C_Register(cfg);
    #endif

    // Start the server with a protocol name and a port
    if (NBN_GameServer_Start("nbnet-example", 42042) < 0)
    {
        NBN_LogError("Failed to start the server");

        // Error, quit the server application
        return 1;

    }
    auto oldTime{std::chrono::high_resolution_clock::now()};
    for(;;){
        auto newTime{std::chrono::high_resolution_clock::now()};
        double diff=std::chrono::duration<double>{newTime-oldTime}.count();
        oldTime=newTime;
        int ev;
        // Poll for server events
        while ((ev = NBN_GameServer_Poll()) != NBN_NO_EVENT)
        {
            if (ev < 0)
            {
                NBN_LogError("Something went wrong");

                // Error, quit the server application
                error = true;
                break;
            }

            switch (ev)
            {
                // New connection request...
                case NBN_NEW_CONNECTION:
                {
                    // Echo server work with one single client at a time
                    if (clients.isFull())
                    {
                        NBN_GameServer_RejectIncomingConnectionWithCode(m_busyCode);
                    }
                    else
                    {
                        NBN_GameServer_AcceptIncomingConnection();
                        clients.addClient(NBN_GameServer_GetIncomingConnection());
                    }

                    break;
                }
                // The client has disconnected
                case NBN_CLIENT_DISCONNECTED:
                {
                    NBN_ConnectionHandle conn = NBN_GameServer_GetDisconnectedClient();
                    NBN_LogInfo("Client has disconnected (id: %d)", conn);
                    clients.removeClient(conn);
                    break;
                }
                // A message has been received from the client
                case NBN_CLIENT_MESSAGE_RECEIVED:
                {
                    clients.shareMessageToOthers(NBN_GameServer_GetMessageInfo());
                    break;
                }
            }
        }
        
        // Pack all enqueued messages as packets and send them
        if (NBN_GameServer_SendPackets() < 0)
        {
            NBN_LogError("Failed to send packets");

            // Error, quit the server application
            error = true;
            break;
        }
        //why is this nessecary on linux?
        std::this_thread::sleep_for(1ms);
    }
}