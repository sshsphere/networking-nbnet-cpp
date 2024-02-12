file(GLOB SOURCES "*.cpp")

if(USE_WEBRTC_DRIVER)
    if (NOT datachannel_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            datachannel
            GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(datachannel)
    endif()
endif()
add_executable(networking-nbnet-server ${SOURCES})

target_link_libraries(networking-nbnet-server raylib)
target_include_directories(networking-nbnet-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(USE_WEBRTC_DRIVER)
    target_link_libraries(networking-nbnet-server datachannel)
    target_compile_definitions(networking-nbnet-server PRIVATE USE_WEBRTC_DRIVER)
endif()