file(GLOB SOURCES "*.cpp")

if (NOT raygui_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raygui
        GIT_REPOSITORY https://github.com/raysan5/raygui.git
        GIT_TAG 4.0
    )
    FetchContent_MakeAvailable(raygui)
endif()

add_executable(networking-nbnet-client ${SOURCES})

if(EMSCRIPTEN)
set_target_properties(networking-nbnet-client PROPERTIES SUFFIX ".html")
add_custom_command(
    TARGET networking-nbnet-client
    POST_BUILD
    COMMAND browserify ${CMAKE_CURRENT_SOURCE_DIR}/../external/net_drivers/webrtc/js/nbnet.js -o nbnet_bundle.js)
endif()

target_link_libraries(networking-nbnet-client raylib)
target_include_directories(networking-nbnet-client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${raygui_SOURCE_DIR}/src)