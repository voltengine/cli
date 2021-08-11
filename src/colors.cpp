#include "colors.hpp"

#include "util/file.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;
using namespace util;

std::unordered_map<std::string, decltype(colors::main)> options {
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

std::ostream &(*success)(std::ostream &) = tc::bright_green;
std::ostream &(*warning)(std::ostream &) = tc::bright_yellow;
std::ostream &(*error  )(std::ostream &) = tc::bright_red;
std::ostream &(*main   )(std::ostream &) = tc::cyan;

void set_from_config() {
	try {
		nl::json config = nl::json::parse(util::read_file(
			std::getenv("VOLT_PATH") / fs::path("config.json")));
		main = options[config["color"]];
	} catch (...) {}
}

}
