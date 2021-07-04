#include "pch.hpp"

#include "command_manager.hpp"

int main(int argc, char **argv) {
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
				if (!cmd->run(args)) {
					std::cout << "Use '"
							<< termcolor::bright_green << "volt"
							<< termcolor::reset << " help "
							<< cmd->name << "' to get help.\n";
				}
			} catch (std::exception &e) {
				std::cout << termcolor::bright_red << "Encountered exception:\n"
						  << termcolor::reset << e.what() << "\n";
			}
		} else
			std::cout << termcolor::bright_red << "No such command.\n"
					  << termcolor::reset << "Use '"
					  << termcolor::bright_green << "volt"
					  << termcolor::reset << " help' to list all available.\n";
	}

	std::cout << std::flush;
	return EXIT_SUCCESS;
}
