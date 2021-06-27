#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include <rapidjson/document.h>
#include <termcolor/termcolor.hpp>

#if _WIN32
	#undef GetObject
	#undef min
#endif
