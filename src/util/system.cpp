#include "system.hpp"

namespace fs = std::filesystem;

namespace util {

void show_terminal_cursor(bool show) {
#ifdef _WIN32
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;

	GetConsoleCursorInfo(out, &info);
	info.bVisible = show;
	SetConsoleCursorInfo(out, &info);
#elif __linux__
	std::cout << (show ? "\033[?25h" : "\033[?25l");
#endif
}

void open_browser(const std::string &url) {
#ifdef _WIN32
	std::system(("start " + url).c_str());
#elif __linux__
	std::system(("URL=\"" + url + "\""
			"&& xdg-open $URL || sensible-browser $URL"
			"|| x-www-browser $URL || gnome-open $URL").c_str());
#endif
}

void start_in_background(const fs::path &path) {
#ifdef _WIN32
	std::system(("start \"\" /D \"" + path.parent_path().string()
			+ "\" \"" + path.string() + '"').c_str());
#elif __linux__
	std::system(('"' + path.string() + "\" &").c_str());
#endif
}

}
