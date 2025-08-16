game:
	cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 
	cmake --build build

debug:
	cmake -B build -G Ninja -S . -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address -g"
	cmake --build build

run:
	./build/game

clean:
	rm -rf build/
