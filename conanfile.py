import os
from conans import ConanFile

class ConanFile(ConanFile):
	settings = "os", "compiler", "build_type", "arch"
	requires = [
		"rapidjson/cci.20200410",
		"termcolor/2.0.0",
		"libgit2/1.1.0"
	]
	generators = "cmake_find_package"

	def imports(self):
		dest = os.getenv('CONAN_IMPORT_PATH', 'bin')
		self.copy('*.dll', dst=dest, src='bin')
