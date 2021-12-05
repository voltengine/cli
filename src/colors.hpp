#pragma once

#include "pch.hpp"

namespace colors {

using _operator = std::ostream &(*)(std::ostream &);

extern _operator success;
extern _operator warning;
extern _operator error;
extern _operator main;

void set_from_config();

}
