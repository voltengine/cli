# Performs a clean release build and places shipping files under "./release".

import os, shutil, time

# Initial clean-up:

if os.path.isdir('release'):
	shutil.rmtree('release')
time.sleep(1)
os.mkdir('release')

# Configure and build:

os.system('cmake -S . -B release/.cache -DCMAKE_BUILD_TYPE=Release')
os.system('cmake --build release/.cache --target volt_cli --config Release')

# Transfer targets:

shutil.copytree(os.path.join('release', '.cache', 'bin'), os.path.join('release', 'bin'))
shutil.rmtree(os.path.join('release', '.cache'))

# Copy Volt.cmake

shutil.copyfile( os.path.join('cmake', 'Volt.cmake'), os.path.join('release', 'Volt.cmake'))
