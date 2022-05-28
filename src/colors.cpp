#include "colors.hpp"

#include "util/file.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;

std::map<std::string, colors::_operator> options {
	{ "red",     tc::red },
	{ "green",   tc::green },
	{ "yellow",  tc::yellow },
	{ "blue",    tc::blue },
	{ "magenta", tc::magenta },
	{ "cyan",    tc::cyan },
	{ "bright-red",     tc::bright_red },
	{ "bright-green",   tc::bright_green },
	{ "bright-yellow",  tc::bright_yellow },
	{ "bright-blue",    tc::bright_blue },
	{ "bright-magenta", tc::bright_magenta },
	{ "bright-cyan",    tc::bright_cyan }
};

namespace colors {

_operator success = tc::bright_green;
_operator warning = tc::bright_yellow;
_operator error = tc::bright_red;
_operator main = tc::cyan;

void set_from_config() {
	try {
		nl::json config = nl::json::parse(util::read_file(
				common::getenv("VOLT_PATH") / fs::path("config.json")));
		main = options[config["color"]];
	} catch (...) {}
}

}
