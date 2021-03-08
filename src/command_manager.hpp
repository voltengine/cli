#pragma once

#include <memory>
#include <string>
#include <vector>

#include "command.hpp"

class command_manager {
public:
	static void init();

	static const std::vector<std::unique_ptr<command>> &get_commands();

	static const std::unique_ptr<command> &find_command(std::string name);
	
private:
	static std::vector<std::unique_ptr<command>> commands;
};
