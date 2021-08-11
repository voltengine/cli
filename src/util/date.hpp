#pragma once

#include "pch.hpp"

namespace util {

date::sys_time<std::chrono::milliseconds>
		parse_iso_date(const std::string &str);

}
