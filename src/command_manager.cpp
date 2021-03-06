#include "command_manager.hpp"

#include "commands/commands.hpp"

using namespace commands;

void command_manager::init() {
	commands["auth"] = std::make_shared<auth_command>();
	commands["build"] = std::make_shared<build_command>();
	commands["debug"] = std::make_shared<debug_command>();
	commands["edit"] = std::make_shared<edit_command>();
	commands["help"] = std::make_shared<help_command>();
	commands["info"] = std::make_shared<info_command>();
	commands["init"] = std::make_shared<init_command>();
	commands["install"] = std::make_shared<install_command>();
	commands["list"] = std::make_shared<list_command>();
	commands["publish"] = std::make_shared<publish_command>();
	commands["search"] = std::make_shared<search_command>();
	commands["top"] = std::make_shared<top_command>();
	commands["uninstall"] = std::make_shared<uninstall_command>();
	commands["unpublish"] = std::make_shared<unpublish_command>();
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
