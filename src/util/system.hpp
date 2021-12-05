#pragma once

#include "pch.hpp"

namespace util {

void show_terminal_cursor(bool show);

void open_browser(const std::string &url);

void start_in_background(const std::filesystem::path &path);

}