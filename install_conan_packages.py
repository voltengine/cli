import os

os.system('conan install -s build_type=Release -if build . --build missing')
input('Press ENTER to continue...')
