#pragma once

#include "pch.hpp"

#include "command.hpp"

class command_manager {
public:
	static void init();

	static const std::unordered_map<std::string, std::shared_ptr<const command>> &get_commands();

	static std::shared_ptr<const command> find_command(const std::string &name);
	
private:
	static std::unordered_map<std::string, std::shared_ptr<const command>> commands;
};
