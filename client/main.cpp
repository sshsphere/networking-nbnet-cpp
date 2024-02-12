#include "NetworkManager.hpp"
#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <iostream>
#include <map>
#include <cassert>

struct Player{
    float x{50}; 
    float y{50};
    Color p{RED};
    void simulate(){
        if(IsKeyDown(KEY_W))y-=200*GetFrameTime();
        if(IsKeyDown(KEY_S))y+=200*GetFrameTime();
        if(IsKeyDown(KEY_A))x-=200*GetFrameTime();
        if(IsKeyDown(KEY_D))x+=200*GetFrameTime();
        if(IsKeyDown(KEY_SPACE))p=Color{static_cast<uint8_t>(rand()%256),static_cast<uint8_t>(rand()%256),static_cast<uint8_t>(rand()%256),255};
    }
    void draw() const{
        DrawCircle(x,y,15.0f,p);
    }
};


class Game{
public:
    struct PlayerInfo{
        Player player{};
        double lastUpdate{};
    };

    std::map<int,PlayerInfo> players{{-1,PlayerInfo{}}};

    void handleMessage(const Message& msg){
        switch(msg.type){
        case NetworkManager::ECHO:{
            std::cout<<"MSG:"<<std::string_view(reinterpret_cast<char *>(msg.data), msg.length)<<'\n';
            break;
        }
        case NetworkManager::PLAYER_DATA:{
            assert(sizeof(Player)==msg.length);
            Player player;
            uint8_t* data = reinterpret_cast<uint8_t*>(&player);
            for (size_t i{0}; i < msg.length; ++i) {
                data[i] = msg.data[i];
            }
            players[msg.clientId]={player,m_time};
            break;
        }

        default:assert(false);
        }
    }
    void simulate(){
        m_time = GetTime();
        
        std::erase_if(players, [&](auto p) {
            return p.first!=-1&&m_time-p.second.lastUpdate>5;
        });
        
        players[-1].player.simulate();
    }
    void draw(){
        for(const auto& p : players)p.second.player.draw();
    }

private:
    double m_time{GetTime()};
};

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    Game game;

    SetTraceLogLevel(LOG_DEBUG);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    constexpr int FPS{60};
    SetTargetFPS(FPS);

    NetworkManager networkManager;

    float dt=1.0f/FPS;
    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawFPS(10,10);

        networkManager.pollEvents(std::bind_front(&Game::handleMessage,&game));

        if(networkManager.isConnected()){

            game.simulate();
            networkManager.sendStruct(NetworkManager::PLAYER_DATA, game.players[-1].player);
            game.draw();
        }

        EndDrawing();
    }

    CloseWindow();
}