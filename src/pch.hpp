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
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <regex>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include <curl/curl.h>
#include <date/date.h>
#include <nlohmann/json.hpp>
#include <termcolor/termcolor.hpp>
