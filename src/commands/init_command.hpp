#pragma once

#include "pch.hpp"

#include "command.hpp"

namespace commands {

class init_command : public command {
public:
	init_command();

	void run(const std::vector<std::string> &args) const override;
};

}
