#pragma once

#include "pch.hpp"

#include "command.hpp"

class command_manager {
public:
	static void init();

	static const std::map<std::string, std::shared_ptr<
			const command>> &get_commands() noexcept;

	static std::shared_ptr<const command>
			find_command(const std::string &name);
	
private:
	static std::map<std::string,
			std::shared_ptr<const command>> commands;
};
