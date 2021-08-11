#pragma once

#include "pch.hpp"

namespace common {

struct authorization_result {
	std::string token;
	nlohmann::json user;
};

std::string get_valid_id(std::string id);

nlohmann::json find_manifest_in_archives(std::string id, bool verbose = true);

// Returns archive URL
std::string select_archive();

// Returns token
std::string get_cached_token(const std::string &archive_url);

nlohmann::json get_user_info(const std::string &token);

// Returns token + user info
authorization_result authorize(const std::string &archive_url);

}
