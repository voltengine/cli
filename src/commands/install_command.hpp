#pragma once

#include "pch.hpp"

#include "command.hpp"

namespace commands {

class install_command : public command {
public:
	install_command();

	void run(const std::vector<std::string> &args) const override;
};

}
