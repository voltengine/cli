#include "common.hpp"

#include "util/file.hpp"
#include "util/http.hpp"
#include "util/string.hpp"
#include "util/system.hpp"
#include "colors.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;
using namespace util;

static std::vector<std::string> anim_frames {
	"_",".","o","O","^","*","-","."
};

static std::map<std::string, std::string> toolchain_suffixes{
	{ "linux-amd64", "LinuxAMD64" },
	{ "windows-amd64", "WindowsAMD64" },
	{ "windows-i686", "WindowsI686" }
};

#ifdef WIN32
	#ifdef _M_X86
static std::map<std::string, std::string> vcvarsall_filenames {
	{ "linux-amd64", "vcvarsx86_amd64.bat" },
	{ "windows-amd64", "vcvars32.bat" },
	{ "windows-i686", "vcvarsx86_amd64.bat" }
};
static std::string default_vcvarsall = "vcvars32.bat";
	#else
static std::map<std::string, std::string> vcvarsall_filenames {
	{ "linux-amd64", "vcvars64.bat" },
	{ "windows-amd64", "vcvars64.bat" },
	{ "windows-i686", "vcvarsamd64_x86.bat" }
};
static std::string default_vcvarsall = "vcvars64.bat";
	#endif
#endif


namespace common {

std::string get_valid_id(std::string id) {
	nl::json config = nl::json::parse(util::read_file(
			common::getenv("VOLT_PATH") / fs::path("config.json")));

	if (id.find('/') == std::string::npos)
		id = config["defaultScope"].get_ref<nl::json::string_t &>() + '/' + id;

	static const std::regex id_validator(
		"(?=^.{1,39}\\/.{1,64}$)^([a-z\\d]+(-[a-z\\d]+)*)\\/"
		"([a-z][a-z\\d]*(-[a-z\\d]+)*)$");

	if (!std::regex_match(id, id_validator))
		throw std::runtime_error("Invalid package ID.");

	return id;
}

nl::json find_manifest_in_archives(std::string id, bool verbose) {
	nl::json manifest;

	nl::json::object_t archives = nl::json::parse(util::read_file(
			common::getenv("VOLT_PATH") / fs::path("config.json"))
			)["archives"];

	for (auto &archive : archives) {
		std::string url = archive.first;
		if (url.back() != '/')
			url += '/';

		if (verbose)
			std::cout << "Checking at \"" << url << "\"..." << std::endl;

		url += "package/" + id + '/';
		try {
			manifest = nl::json::parse(util::download(url));
			break;
		} catch (std::exception &e) {
			if (verbose) {
				std::cout << colors::warning
				          << "Not found: " << e.what() << '\n'
				          << tc::reset;
			}
		}
	}

	if (manifest.is_null())
		throw std::runtime_error("Package not found in archives.");

	return manifest;
}

std::string select_archive() {
	fs::path volt_path = common::getenv("VOLT_PATH");
	fs::path config_path = volt_path / "config.json";

	nl::json::object_t archives = nl::json::parse(
			util::read_file(config_path))["archives"];

	if (archives.size() == 0)
		throw std::runtime_error("Archive list is empty.");

	size_t archive_index = 0;
	if (archives.size() != 1) {
		std::cout << colors::main
		          << "Multiple archives are available:\n"
		          << tc::reset;

		for (auto it = archives.begin(); it != archives.end(); it++) {
			std::cout << colors::main << '['
			          << tc::reset << std::distance(archives.begin(), it)
			          << colors::main << "]: "
			          << tc::reset << it->first
			          << '\n';
		}

		while (true) {
			std::cout << colors::main
			          << "Selection: "
			          << tc::reset;

			std::string line;
			std::getline(std::cin, line);
			if (std::cin.fail())
				std::exit(EXIT_SUCCESS);

			try {
				archive_index = std::stoull(line);
				if (archive_index >= archives.size())
					throw std::out_of_range("");
				break;
			} catch (...) {
				std::cout << colors::error
				          << "Invalid input.\n"
				          << tc::reset;
			}
		}

		std::cout << '\n';
	}

	auto it = archives.begin();
	std::advance(it, archive_index);
	std::string url = it->first;
	if (url.back() != '/')
		url += '/';

	return url;
}

std::string get_cached_token(const std::string &archive_url) {
	fs::path volt_path = common::getenv("VOLT_PATH");
	fs::path config_path = volt_path / "config.json";

	nl::json config = nl::json::parse(util::read_file(config_path));
	return config["archives"][archive_url];
}

nl::json get_user_info(const std::string &token) {
	fs::path volt_path = common::getenv("VOLT_PATH");
	fs::path cert_path = volt_path / "cacert.pem";

	if (token.empty())
		return nl::json();

	std::string buffer;
	nl::json response;
	http request;

	request.set_certificate(cert_path);
	request.set_method(http::method::get);
	request.set_url("https://api.github.com/user");
	request.set_header("Accept", "application/vnd.github.v3+json");
	request.set_header("Authorization", "Bearer " + token);
	request.on_response([](const auto &response) {
		if (response.status != 200 && response.status != 401) {
			throw http::error("Remote returned " +
					std::to_string(response.status) + ".");
		}
	});
	request.on_data([&buffer](const auto &data) {
		buffer.insert(buffer.end(), data.begin(), data.end());
	});

	request.send();
	response = nl::json::parse(buffer);
	buffer.clear();

	if (response.contains("login"))
		return response;

	return nl::json();
}
 
authorization_result authorize(const std::string &archive_url) {
	fs::path volt_path = common::getenv("VOLT_PATH");
	fs::path config_path = volt_path / "config.json";
	fs::path cert_path = volt_path / "cacert.pem";

	std::cout << "Fetching client ID...\n";

	std::string client_id = util::download(archive_url + "auth/id/");

	std::string buffer;
	nl::json response;
	http request;

	request.set_certificate(cert_path);
	request.set_method(http::method::post);
	request.set_header("Content-Type", "application/x-www-form-urlencoded");
	request.set_header("Accept", "application/vnd.github.v3+json");
	request.on_response([](const auto &response) {
		if (response.status != 200) {
			throw http::error("Remote returned " +
					std::to_string(response.status) + ".");
		}
	});
	request.on_data([&buffer](const auto &data) {
		buffer.insert(buffer.end(), data.begin(), data.end());
	});

	request.set_url("https://github.com/login/device/code");
	request.set_body("scope=read:org&client_id=" + client_id);

	request.send();
	response = nl::json::parse(buffer);
	buffer.clear();

	std::string device_code = response["device_code"];
	uint32_t expires_in = response["expires_in"];
	uint32_t interval = response["interval"].get<uint32_t>() + 1;
	std::string user_code = response["user_code"];
	std::string verification_uri = response["verification_uri"];

	std::cout << "\nPlease enter your code at verification URL:"
	          << colors::main
	          << "\nCode: " << tc::reset << user_code
	          << colors::main
	          << "\nURL: " << tc::reset << verification_uri
	          << colors::main
	          << "\n\nCode will remain valid for the next "
	          << std::round(expires_in / 60.0) << " minutes."
	          << tc::reset << "\n\n";
	
	request.set_url("https://github.com/login/oauth/access_token");
	request.set_body("client_id=" + client_id
			+ "&device_code=" + device_code
			+ "&grant_type=urn:ietf:params:oauth:grant-type:device_code");

	authorization_result result;

	util::show_terminal_cursor(false);
	try {
		uint32_t seconds = 0, frame = -1;
		auto start_time = std::chrono::system_clock::now();
		auto last_checked = start_time;
		bool opened_browser = false;
		while (true) {
			std::cout << colors::main
			          << anim_frames[frame = (frame + 1) % anim_frames.size()]
			          << tc::reset << " Waiting for authorization...\r";

			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			auto now = std::chrono::system_clock::now();
			uint32_t seconds_from_start = std::chrono::duration_cast<std::
					chrono::seconds>(now - start_time).count();
			
			if (!opened_browser && seconds_from_start > 1) {
				util::open_browser(verification_uri);
				opened_browser = true;
			}

			if (seconds_from_start > expires_in)
				throw std::runtime_error("Code has expired.");

			if (std::chrono::duration_cast<std::chrono::seconds>(
					now - last_checked).count() < interval)
				continue;
			last_checked = now;

			request.send();
			response = nl::json::parse(buffer);
			buffer.clear();

			if (response.contains("access_token"))
				break;
			
			if (response.contains("interval"))
				interval = response["interval"].get<uint32_t>() + 1;
		}

		result.token = response["access_token"];
		result.user = get_user_info(result.token);

		if (result.user.is_null())
			throw std::runtime_error("Received invalid token.");

		std::cout << "Authorized as "
		          << colors::main
		          << result.user["login"].get_ref<nl::json::string_t &>()
		          << tc::reset << ".               \n";
	} catch (std::exception &e) {
		std::cout << "Authorization failed.         \n";
		util::show_terminal_cursor(true);
		throw e;
	}
	util::show_terminal_cursor(true);

	nl::json config = nl::json::parse(util::read_file(config_path));
	config["archives"][archive_url] = result.token;
	util::write_file(config_path, config.dump(1, '\t'));

	std::cout << colors::success << "\nFile was written:\n"
	          << tc::reset << config_path.string() << '\n';

	return result;
}

void cmake_build(
		const fs::path &build_path,
		const std::string &platform,
		bool development, bool debug) {
	auto build_path_str = build_path.string();

	// Get path to the CMake toolchain
	fs::path volt_path = common::getenv("VOLT_PATH");
	auto toolchains_path = volt_path / "cmake" / "toolchains";

	std::string toolchain_path_str;
	if (!platform.empty()) {
		if (!toolchain_suffixes.contains(platform))
			throw std::runtime_error("Invalid platform: " + platform);

		fs::path toolchain_path = toolchains_path / (VOLT_CLI_TOOLCHAIN_PREFIX
			"-" + toolchain_suffixes[platform] + ".cmake");
		toolchain_path_str = toolchain_path.string();
	} else {
		fs::path toolchain_path = toolchains_path / (VOLT_CLI_TOOLCHAIN_PREFIX
			"-" VOLT_CLI_TOOLCHAIN_PREFIX ".cmake");
		toolchain_path_str = toolchain_path.string();
	}

#ifdef _WIN32
	util::replace(build_path_str, "\\", "\\\\");
	util::replace(toolchain_path_str, "\\", "\\\\");

	const char *comn_tools = common::getenv("VS170COMNTOOLS");
	if (comn_tools == nullptr)
		throw std::runtime_error("Visual Studio 2022 is not installed.");

	fs::path vcvarsall_path = fs::path(comn_tools) / "..\\..\\VC\\Auxiliary\\Build";
	vcvarsall_path /= platform.empty() ? default_vcvarsall : vcvarsall_filenames[platform];
	std::string vcvarsall_cmd_prefix = "\"" + vcvarsall_path.string() + "\" && ";
#endif

	std::string cmd = "cmake -S . -B \"" + build_path_str + "\" -G Ninja --no-warn-unused-cli"
			" -D CMAKE_BUILD_TYPE=" + (debug ? "Debug" : "Release");
	if (!toolchain_path_str.empty())
		cmd += " -D CMAKE_TOOLCHAIN_FILE=\"" + toolchain_path_str + '"';
	if (!development)
		cmd += " -D VOLT_DEVELOPMENT=OFF";

#ifdef _WIN32
	cmd = vcvarsall_cmd_prefix + cmd;
#endif

	std::cout << colors::main << "CMake configuration:\n" << tc::reset;
	
	std::cout << "> " << cmd << "\n\n";
	if (util::shell(cmd, [](std::string_view out) {
		std::cout << out;
	}) != 0)
		throw std::runtime_error("CMake configuration failed.");

	cmd = "cmake --build \"" + build_path_str
			+ "\" --config " + (debug ? "Debug" : "Release") +
			+ " -j " + util::to_string(std::thread::hardware_concurrency() + 2)
			+ " --target all";

#ifdef _WIN32
	cmd = vcvarsall_cmd_prefix + cmd;
#endif

	std::cout << colors::main << "\nCMake build:\n" << tc::reset;

	std::cout << "> " << cmd << "\n\n";
	if (util::shell(cmd, [](std::string_view out) {
		std::cout << out;
	}) != 0)
		throw std::runtime_error("CMake build failed.");
}

fs::path copy_cmake_output_binaries(
		const std::filesystem::path &build_path,
		const std::filesystem::path &target_path) {
	fs::path current_path = fs::current_path();
	auto package = nl::json::parse(util::read_file(
			current_path / "package.json"));
	
	auto id = package["id"].get_ref<nl::json::string_t &>();
	auto app_name = id.substr(id.find('/') + 1);

	auto build_bin = build_path / "bin";
	auto target_bin = target_path / "bin";

	fs::create_directories(target_bin);

	std::cout << colors::main << "\nCopying binaries:\n" << tc::reset;

	for (auto &item : fs::directory_iterator(build_bin)) {
		auto path = item.path();
		auto ext = path.extension();
		if (ext != ".exe" && ext != ".dll"
				&& ext != "" && ext != ".so")
			continue;

		auto name = path.filename();
		if (name.stem() == "APP") {
			std::cout << name.string() << " -> ";
			name = (app_name += name.extension().string());
		}
		
		try {
			fs::copy(path, target_bin / name);
			std::cout << name.string() << '\n';
		} catch (...) {
			std::cout << colors::error << name.string() << tc::reset << '\n';
		}
	}

	return target_bin / app_name;
}

std::string getenv(const std::string &name) {
	char *env = std::getenv(name.data());
	if (env == nullptr)
		throw std::runtime_error("Failed to read environment variable \"" + name + "\".");
	return env;
}

}
