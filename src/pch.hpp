#pragma once

#if _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX

	#include <Windows.h>
	
	#undef GetObject
#endif

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cctype>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include <curl/curl.h>
#include <sodium.h>
#include <rapidjson/error/en.h>
#include <rapidjson/document.h>
#include <termcolor/termcolor.hpp>
