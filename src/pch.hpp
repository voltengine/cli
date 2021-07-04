#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include <rapidjson/document.h>
#include <termcolor/termcolor.hpp>
#include <git2.h>

#if _WIN32
	#undef GetObject
	#undef min
#endif
