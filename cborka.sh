rm -r build/ | rm -r Release
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
cmake --install build --prefix ./Release --config Release