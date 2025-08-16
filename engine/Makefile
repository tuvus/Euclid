engine:
	cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1
	cmake --build build

test: engine
	./build/tests/Test

clean:
	rm -rf build/
