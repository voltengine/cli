#pragma once

#include "command.hpp"

class help_command : public command {
public:
	help_command();

	void run(const std::vector<std::string> &args) const override;
};
