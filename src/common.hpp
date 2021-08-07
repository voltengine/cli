#pragma once

#include "pch.hpp"

#include "util/json.hpp"

namespace common {

struct authorization_result {
	std::string token;
	util::json user;
};

std::string prepend_default_scope(std::string_view id);

util::json find_manifest_in_archives(std::string id);

// Returns archive URL
std::string select_archive();

// Returns token
std::string get_cached_token(const std::string &archive_url);

util::json get_user_info(const std::string &token);

// Returns token + user info
authorization_result authorize(const std::string &archive_url);

}
