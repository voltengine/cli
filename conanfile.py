import os
from conans import ConanFile

class ConanFile(ConanFile):
	settings = "os", "compiler", "build_type", "arch"
	requires = [
		"date/3.0.1",
		"libcurl/7.77.0",
		"rapidjson/cci.20200410",
		"termcolor/2.0.0"
	]
	generators = "cmake_find_package"

	def imports(self):
		dest = os.getenv('CONAN_IMPORT_PATH', 'bin')
		self.copy('*.dll', dst=dest, src='bin')
