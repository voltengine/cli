#pragma once

#include "pch.hpp"

namespace util {

std::string read_file(const std::filesystem::path &path);

void write_file(const std::filesystem::path &path, std::string_view str);

std::string download(std::string_view url);

void shell(std::string cmd, const std::function<void(
		std::string_view)> &stdout_cb, size_t buffer_capacity = 1024);

}
