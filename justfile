
configure:
    conan install . --output-folder=build --build=missing
    source build/build/Release/generators/conanbuild.sh && cmake --preset conan-release
    ln -sf build/build/Release/compile_commands.json ./compile_commands.json

build:
	source build/build/Release/generators/conanbuild.sh && cmake --build --preset conan-release

cbuild:
	rm -rf build generated
	mkdir generated
	just configure
	just build

server:
	./build/build/Release/server

client:
	./build/build/Release/client

dev-server:
    just cbulid
    just server

dev-client:
    just cbuild
    just client