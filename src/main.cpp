#include "pch.hpp"

#include "util/system.hpp"
#include "colors.hpp"
#include "command_manager.hpp"

namespace tc = termcolor;

int main(int argc, char **argv) {
	std::signal(SIGINT, [](int) {
		std::cout << tc::reset;
		util::show_terminal_cursor(true);
		
		std::exit(EXIT_SUCCESS);
	});

	colors::set_from_config();
	command_manager::init();
	
	std::vector<std::string> args;

	if (argc == 1)
		command_manager::find_command("help")->run(args);
	else {
		std::string name(argv[1]);
		args.resize(argc - 2);
		for (int i = 0; i < argc - 2; i++)
			args[i] = argv[i + 2];
		
		const auto &cmd = command_manager::find_command(name);
		if (cmd) {
			try {
				cmd->run(args);
			} catch (std::exception &e) {
				std::cout << colors::error
				          << '\n' << e.what() << '\n'
				          << tc::reset;
			}
		} else
			std::cout << colors::error << "No such command.\n"
			          << tc::reset << "Use \""
			          << colors::main << "volt"
			          << tc::reset << " help\" to list all available.\n";
	}

	std::cout << std::flush;
	return EXIT_SUCCESS;
}
