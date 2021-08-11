#include "commands.hpp"

#include "colors.hpp"
#include "common.hpp"

namespace tc = termcolor;

namespace commands {

auth_command::auth_command() : command(
		"auth",
		"",
		"Authenticates user to selected archive and "
		"stores access token for later use.") {}

void auth_command::run(const std::vector<std::string> &args) const {
	common::authorize(common::select_archive());
}

}
