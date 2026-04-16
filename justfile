
configure:
    cmake -B build
    ln -sf build/compile_commands.json .

build:
    cmake --build build

cbuild:
    rm -rf build generated
    mkdir generated
    cmake -B build
    cmake --build build
    ln -sf build/compile_commands.json .

server:
    ./build/server

client:
    ./build/client

dev-server:
    just cbulid
    ./build/server

dev-client:
    just cbuild
    ./build/client