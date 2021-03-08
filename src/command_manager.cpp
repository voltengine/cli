#include "command_manager.hpp"

#include <algorithm>

#include "help_command.hpp"

template<typename T>
std::unique_ptr<command> make_command() {
	return std::unique_ptr<command>(static_cast<command *>(new T));
}

void command_manager::init() {
	commands.push_back(make_command<help_command>());
}

const std::vector<std::unique_ptr<command>> &command_manager::get_commands() {
	return commands;
}

const std::unique_ptr<command> &command_manager::find_command(std::string name) {
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	
	auto it = std::find_if(commands.begin(), commands.end(),
			[name](const auto &cmd) { return cmd->name == name; });

	if (it != commands.end())
		return *it;
	else
		return nullptr;
}

std::vector<std::unique_ptr<command>> command_manager::commands;
