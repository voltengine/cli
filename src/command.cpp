#include "command.hpp"

command::command(std::string_view name,
		std::string_view syntax,
		std::string_view description)
		: name(name), syntax(syntax), description(description) {}
		