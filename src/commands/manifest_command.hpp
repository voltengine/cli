#pragma once

#include "pch.hpp"

#include "command.hpp"

namespace commands {

class manifest_command : public command {
public:
	manifest_command();

	void run(const std::vector<std::string> &args) const override;
};

}
