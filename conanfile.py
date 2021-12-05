import os
from conans import ConanFile

class ConanFile(ConanFile):
	settings = 'os', 'compiler', 'build_type', 'arch'
	requires = [
		# 'date/3.0.1',
		# Parts of date 3.0.1 are incompatible with Windows SDK 10.0.19041.0
		# and cause template parameter deduction errors with date::parse / date::parse
		# Temporarily falling back to local installation
		'libcurl/7.77.0',
		'nlohmann_json/3.9.1',
		'termcolor/2.0.0'
	]
	generators = 'cmake_find_package'

	def imports(self):
		dest = os.getenv('CONAN_IMPORT_PATH', 'bin')
		self.copy('*.dll', dst=dest, src='bin')
