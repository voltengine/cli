#include "command_manager.hpp"

#include "commands/help_command.hpp"
#include "commands/info_command.hpp"
#include "commands/init_command.hpp"
#include "commands/install_command.hpp"
#include "commands/list_command.hpp"
#include "commands/search_command.hpp"

using namespace commands;

void command_manager::init() {
	commands["help"] = std::make_shared<help_command>();
	commands["info"] = std::make_shared<info_command>();
	commands["init"] = std::make_shared<init_command>();
	// commands["install"] = std::make_shared<install_command>();
	commands["list"] = std::make_shared<list_command>();
	commands["search"] = std::make_shared<search_command>();
}

const std::map<std::string, std::shared_ptr<
		const command>> &command_manager::get_commands() noexcept {
	return commands;
}

std::shared_ptr<const command> command_manager::
		find_command(const std::string &name) {
	if (commands.contains(name))
		return commands[name];
	else
		return nullptr;
	
}

std::map<std::string, std::shared_ptr<
		const command>> command_manager::commands;
