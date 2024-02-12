cmake_minimum_required(VERSION 3.20)
project(networking-nbnet VERSION 0.1.0)
set(OPENSSL_ROOT_DIR "D:\\dev\\openssl")
option(BUILD_CLIENT "Build the client target" OFF)
option(BUILD_SERVER "Build the server target" ON)
option(USE_WEBRTC_DRIVER "Build the server with WebRTC C driver, which allows for both native and web clients to connect to the same server (linux only)" ON)
option(EMSCRIPTEN "Build for web" OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CTest)
enable_testing()
set(CMAKE_CXX_STANDARD 20)

set(GRAPHICS GRAPHICS_API_OPENGL_43)
if(EMSCRIPTEN)
    set(PLATFORM Web)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY")
endif()

if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 4.5.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    link_libraries(ws2_32)
else()
    link_libraries(pthread)
endif()

if (EMSCRIPTEN)
    set(ASYNCIFY_IMPORTS "[\"__js_game_server_start\", \"__js_game_client_start\", \"__js_game_client_close\"]")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_MEMORY=30MB -s EXIT_RUNTIME=1 -s ASSERTIONS=1 -s ASYNCIFY -s ASYNCIFY_IMPORTS=\"${ASYNCIFY_IMPORTS}\"")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --js-library ${CMAKE_CURRENT_SOURCE_DIR}/external/net_drivers/webrtc/js/api.js")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3 --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html")
endif()

if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

if(BUILD_SERVER)
    add_subdirectory(server)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
