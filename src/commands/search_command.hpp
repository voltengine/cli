#pragma once

#include "pch.hpp"

#include "command.hpp"

namespace commands {

class search_command : public command {
public:
	search_command();

	bool run(const std::vector<std::string> &args) const override;
};

}
