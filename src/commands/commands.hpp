#pragma once

#include "pch.hpp"

#include "command.hpp"

#define DECLARE_COMMAND(class_name) \
		class class_name : public command { \
		public: \
			class_name(); \
\
			void run(const std::vector<std::string> &args) const override; \
		};

namespace commands {

DECLARE_COMMAND(auth_command);
DECLARE_COMMAND(build_command);
DECLARE_COMMAND(debug_command);
DECLARE_COMMAND(edit_command);
DECLARE_COMMAND(help_command);
DECLARE_COMMAND(info_command);
DECLARE_COMMAND(init_command);
DECLARE_COMMAND(install_command);
DECLARE_COMMAND(list_command);
DECLARE_COMMAND(publish_command);
DECLARE_COMMAND(search_command);
DECLARE_COMMAND(top_command);
DECLARE_COMMAND(uninstall_command);
DECLARE_COMMAND(unpublish_command);

}

#undef DECLARE_COMMAND
