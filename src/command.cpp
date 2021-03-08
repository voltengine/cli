#include "command.hpp"

command::command(const std::string &name,
		const std::string &syntax,
		const std::string &description)
		: name(name), syntax(syntax), description(description) {}
		