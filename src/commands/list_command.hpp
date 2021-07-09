#pragma once

#include "pch.hpp"

#include "command.hpp"

namespace commands {

class list_command : public command {
public:
	list_command();

	void run(const std::vector<std::string> &args) const override;
};

}
