#pragma once

#include "pch.hpp"

namespace util {

std::string read_file(const std::filesystem::path &path);

void write_file(const std::filesystem::path &path, std::string_view str);

std::string download(std::string_view url,
		const std::filesystem::path &https_certificate = {});

}
