#include "pch.hpp"

#include "command_manager.hpp"
#include "util/version.hpp"
#include <random>

int main(int argc, char **argv) {
	// command_manager::init();
	// std::vector<std::string> args;

	// if (argc == 1)
	// 	command_manager::find_command("help")->run(args);
	// else {
	// 	std::string name(argv[1]);
	// 	args.resize(argc - 2);
	// 	for (int i = 0; i < argc - 2; i++)
	// 		args[i] = argv[i + 2];
		
	// 	const auto &cmd = command_manager::find_command(name);
	// 	if (cmd)
	// 		cmd->run(args);
	// 	else
	// 		std::cout << termcolor::red << "No such command.\n"
	// 				  << termcolor::reset << "Use 'volt help' to list all available.\n";
	// }

	// std::cout << std::flush;
	// return EXIT_SUCCESS;

	// 1.0.0-alpha < 1.0.0-alpha.1 < 1.0.0-alpha.beta < 1.0.0-beta < 1.0.0-beta.2 < 1.0.0-beta.11 < 1.0.0-rc.1 < 1.0.0

	std::vector<util::version> v {
		util::version("1.0.0-alpha"),
		util::version("1.0.0-alpha.1"),
		util::version("1.0.0-alpha.beta"),
		util::version("1.0.0-beta"),
		util::version("1.0.0-beta.2"),
		util::version("1.0.0-beta.11"),
		util::version("1.0.0-rc.1"),
		util::version("1.0.0")
	};

    std::mt19937 g(std::random_device{}());
	std::shuffle(v.begin(), v.end(), g);

	for (auto &ver : v)
		std::cout << ver << std::endl;

	std::cout << std::endl;

	std::sort(v.begin(), v.end());

	for (auto &ver : v)
		std::cout << ver << std::endl;
}
