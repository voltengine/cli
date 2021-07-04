#pragma once

#include "pch.hpp"

#include "command.hpp"

namespace commands {

class info_command : public command {
public:
	info_command();

	bool run(const std::vector<std::string> &args) const override;
};

}
