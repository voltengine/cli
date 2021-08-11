#pragma once

#include "pch.hpp"

namespace colors {

extern std::ostream &(*success)(std::ostream &);
extern std::ostream &(*warning)(std::ostream &);
extern std::ostream &(*error  )(std::ostream &);
extern std::ostream &(*main   )(std::ostream &);

void set_from_config();

}
