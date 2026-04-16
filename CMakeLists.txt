cmake_minimum_required(VERSION 3.15)
project(Dolos C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(${GEN_DIR} ${GSOAP_INCLUDE_DIR})

set(SERVICE           ${CMAKE_SOURCE_DIR}/services/sayhello.h)
set(GEN_DIR           ${CMAKE_SOURCE_DIR}/generated)
set(GSOAP_INCLUDE_DIR /opt/homebrew/Cellar/gsoap/2.8.141/include)
set(GSOAP_RUNTIME     ${CMAKE_SOURCE_DIR}/stdsoap2.c)

file(MAKE_DIRECTORY ${GEN_DIR})

add_custom_command(
    OUTPUT
        ${GEN_DIR}/soapC.c
        ${GEN_DIR}/soapServer.c
        ${GEN_DIR}/soapClient.c
        ${GEN_DIR}/soapStub.h
        ${GEN_DIR}/soapH.h
        ${GEN_DIR}/sayhello.nsmap
    COMMAND soapcpp2 -c -d ${GEN_DIR} -I ${GSOAP_INCLUDE_DIR} ${SERVICE}
    DEPENDS ${SERVICE}
)

add_custom_target(gsoap_gen ALL
    DEPENDS ${GEN_DIR}/soapC.c
)

add_executable(server
    server/main.c
    server/server.c
    ${GEN_DIR}/soapC.c
    ${GEN_DIR}/soapServer.c
    ${GSOAP_RUNTIME}
)
target_include_directories(server PRIVATE ${GEN_DIR} ${GSOAP_INCLUDE_DIR} server)
add_dependencies(server gsoap_gen)

add_executable(client
    client/main.c
    client/client.c
    ${GEN_DIR}/soapC.c
    ${GEN_DIR}/soapClient.c
    ${GSOAP_RUNTIME}
)
target_include_directories(client PRIVATE ${GEN_DIR} ${GSOAP_INCLUDE_DIR} client)
add_dependencies(client gsoap_gen)