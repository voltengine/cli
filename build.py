import os

os.system('conan install -s build_type=Release -if build .')
os.system('cmake -S . -B build -DCMAKE_BUILD_TYPE=Release')
os.system('cmake --build build --target volt_cli --config Release')