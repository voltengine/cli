#include "command_manager.hpp"

#include "commands/help_command.hpp"
#include "commands/list_command.hpp"

using namespace commands;

void command_manager::init() {
	commands["help"] = std::make_shared<help_command>();
	commands["list"] = std::make_shared<list_command>();
}

const std::unordered_map<std::string, std::shared_ptr<
		const command>> &command_manager::get_commands() {
	return commands;
}

std::shared_ptr<const command> command_manager::
		find_command(const std::string &name) {
	if (commands.contains(name))
		return commands[name];
	else
		return nullptr;
	
}

std::unordered_map<std::string, std::shared_ptr<
		const command>> command_manager::commands;
