#pragma once

#include "pch.hpp"

#include "command.hpp"

namespace commands {

class help_command : public command {
public:
	help_command();

	void run(const std::vector<std::string> &args) const override;
};

}
