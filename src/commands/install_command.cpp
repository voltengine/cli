#include "commands.hpp"

#include "util/date.hpp"
#include "util/file.hpp"
#include "util/http.hpp"
#include "util/string.hpp"
#include "util/version.hpp"
#include "colors.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;
using namespace util;

class dependency {
public:
	std::string scope, name;
	util::version version;
	std::weak_ptr<dependency> dependant;
	std::vector<std::shared_ptr<dependency>> dependencies;
	std::vector<std::string> warnings;

	std::string get_str() const;

	std::string get_id() const;

	void set_id(std::string_view id);
};

std::string dependency::get_str() const {
	return get_id() + ' ' + util::to_string(version);
}

std::string dependency::get_id() const {
	return scope + '/' + name;
}

void dependency::set_id(std::string_view id) {
	size_t i = id.find('/');
	scope = id.substr(0, i);
	name = id.substr(i + 1);
}

namespace commands {

install_command::install_command() : command(
		"install",
		"[{id} [{version}]]",
		"Checks for dependency conflicts and downloads missing components.\n"
		"Optionally adds a dependency to \"package.json\".") {}

void install_command::run(const std::vector<std::string> &args) const {
	if (args.size() > 2) {
		std::cout << termcolor::bright_yellow << "Ignoring extra arguments.\n"
				  << termcolor::reset;
	}

	if (args.size() > 1)
		util::version(args[1]);

	fs::path package_path = fs::current_path() / "package.json";

	if (!fs::exists(package_path))
		throw std::runtime_error("No \"package.json\" in this directory.");

	nl::json package = nl::json::parse(read_file(package_path));

	// If package ID was specified
	if (args.size() > 0) {
		std::string id = common::get_valid_id(args[0]);
		std::string name = id.substr(id.find('/') + 1);
		
		auto &deps = package["dependencies"].get_ref<nl::json::object_t &>();
		if (std::find_if(deps.begin(), deps.end(), [&name](auto &item) {
			return item.first.substr(item.first.find('/') + 1) == name;
		}) != deps.end())
			throw std::runtime_error("Dependency is already installed.");

		nl::json::object_t releases = common::find_manifest_in_archives(id)["releases"];
		if (releases.empty())
			throw std::runtime_error("Dependency has no releases.");

		std::string release;
		if (args.size() > 1) {
			release = args[1];

			if (!releases.contains(release))
				throw std::runtime_error("No matching releases found.");
		} else {
			release = std::max_element(releases.begin(),
					releases.end(), [](auto &a, auto &b) {
				util::version ver1 = a.first, ver2 = b.first;
				switch (ver1.compare(ver2)) {
				case -1:
					return true;
				case 1:
					return false;
				default:
					return util::parse_iso_date(a.second["created"])
						 < util::parse_iso_date(b.second["created"]);
				}
			})->first;
		}

		package["dependencies"][id] = release;
		util::write_file(package_path, package.dump(1, '\t'));
		std::cout << colors::success << "\nFile has been written:\n"
				  << tc::reset << package_path.string() << "\n\n";
	}

	std::cout << "Building dependency tree...\n";

	auto root = std::make_shared<dependency>();
	root->set_id(package["id"]);
	root->version = util::version(package["version"]);

	// Parent + deps to process and attach to it
	std::stack<std::pair<std::shared_ptr<dependency> &, nl::json::object_t>> pkgs_to_check;
	pkgs_to_check.emplace(root, package["dependencies"]);

	std::unordered_map<std::string, nl::json> manifest_cache;

	while (!pkgs_to_check.empty()) {
		auto pkg = std::move(pkgs_to_check.top());
		pkgs_to_check.pop();

		for (auto &dep : pkg.second) {
			// Add child dependencies to the tree
			size_t id_slash_index = dep.first.find('/');

			auto node = std::make_shared<dependency>();
			node->scope = dep.first.substr(0, id_slash_index);
			node->name = dep.first.substr(id_slash_index + 1);
			node->version = dep.second.get_ref<nl::json::string_t &>();
			node->dependant = pkg.first;

			// Detect circular references

			std::vector<dependency *> parents;
			parents.push_back(pkg.first.get());

			while (true) {
				auto &parent_node = *parents.back();

				if (parent_node.name == node->name) {
					std::ostringstream error;

					if (parent_node.scope == node->scope)
						error << "Circular reference:\n";
					else {
						error << "Circular reference with ambiguous scope:\n";
					}

					for (auto it = parents.rbegin(); it != parents.rend(); it++)
						error << (*it)->get_str() << " -> ";
					error << dep.first << ' ' << node->version;

					throw std::runtime_error(error.str());
				}

				if (auto parent = parent_node.dependant.lock())
					parents.push_back(parent.get());
				else
					break;
			}

			std::cout << "Checking "
					  << colors::main << node->scope
					  << tc::reset << '/'
					  << colors::main << node->name
					  << tc::reset << ' ' << node->version << "...\n";

			// Add child dependencies to the tree

			nl::json *manifest;
			if (!manifest_cache.contains(dep.first)) {
				manifest = &manifest_cache.emplace(dep.first,
						common::find_manifest_in_archives(dep.first, false)).first->second;
			} else
				manifest = &manifest_cache[dep.first];

			if (!(*manifest)["releases"].contains(dep.second)) {
				pkg.first->warnings.push_back("Package " + pkg.first->get_str()
						  + " specifies non-existent release "
						  + dep.second.get_ref<nl::json::string_t &>()
						  + " of " + dep.first + '.');
				continue;
			}

			pkg.first->dependencies.push_back(std::move(node));

			// Queue dependencies object of that dependency to be also processed

			pkgs_to_check.emplace(pkg.first->dependencies.back(), (*manifest)["releases"]
					[dep.second.get_ref<nl::json::string_t &>()]["dependencies"]);
		}
	}

	/* {
		std::cout << "\nTree structure:\n";

		std::stack<std::pair<uint32_t, dependency &>> nodes;
		nodes.emplace(0, *root);
		while (!nodes.empty()) {
			auto node = nodes.top();
			nodes.pop();

			std::cout << std::string(node.first, '\t');
			std::cout << node.second.get_id() << '\n';

			for (auto &x : node.second.dependencies)
				nodes.emplace(node.first + 1, *x);
		}
	} */

	std::cout << "\nApplying overrides...\n";

	// Traversal order matters both during
	// applying overrides or flattening the tree.
	// In the first case it ensures that no warnings
	// for overridden overrides will be shown to the user.
	// And as for the latter - see further down.
	std::queue<dependency *> nodes_to_visit;

	// Can't push root, because each
	// node we visit must have a parent
	for (auto &child : root->dependencies)
		nodes_to_visit.push(child.get());
	
	while (!nodes_to_visit.empty()) {
		auto node = nodes_to_visit.front();
		nodes_to_visit.pop();

		bool overridden = false;

		auto owner = node->dependant.lock();
		auto parent = owner->dependant.lock();
		while (parent) {
			// If any of the parents on the way to the
			// root depends on another version of this node
			auto override = std::find_if(parent->dependencies.begin(),
					parent->dependencies.end(), [&node](auto &item) {
						return item->name == node->name;
					});
			if (override != parent->dependencies.end()) {
				util::version &new_ver = (*override)->version;
				util::version &old_ver = node->version;

				if ((*override)->scope != node->scope) {
					std::string warning = parent->get_str()
							  + " overrides " + node->get_str()
							  + " required by " + owner->get_str()
							  + " with " + (*override)->get_str() + ":\n"
							  "Scope was overwritten, now it is "
							  "a completely different package.";

					owner->warnings.push_back(warning);
				} else if (new_ver.major != old_ver.major || new_ver < old_ver) {
					std::string warning = parent->get_str()
							  + " forces " + node->get_id()
							  + " to " + util::to_string((*override)->version)
							  + " from " + util::to_string(node->version)
							  + " required by " + owner->get_str() + ":\n";

					if (new_ver.major != old_ver.major) {
						warning += "Major version was changed, incompatible "
								"API will be devastating.";
					} else if (new_ver.minor != old_ver.minor) {
						warning += "Minor version was downgraded, some "
								"required features are not be available.";
					} else if (new_ver.patch != old_ver.patch) {
						warning += "Patch version was downgraded, some "
								"required bug-fixes are not present.";
					} else {
						warning += "Pre-release tag was downgraded, some "
								"required bug-fixes are not available.";
					}

					owner->warnings.push_back(warning);
				}

				overridden = true;
				owner->dependencies.erase(std::find_if(
						owner->dependencies.begin(),
						owner->dependencies.end(), [&node](auto &item) {
							return item.get() == node;
						}));
			}
			parent = parent->dependant.lock();
		}

		if (!overridden) {
			for (auto &child : node->dependencies)
				nodes_to_visit.push(child.get());
		}
	}

	std::cout << "\nFlattening dependency tree...\n";
	std::vector<dependency *> final_nodes;

	// This is a queue. Equal versions with different build metadata
	// tags will compare more important for packages higher
	// in the tree or for packages that precede others
	// on the dependency list if they are on the same tree level.
	for (auto &child : root->dependencies)
		nodes_to_visit.push(child.get());
	
	while (!nodes_to_visit.empty()) {
		auto node = nodes_to_visit.front();
		nodes_to_visit.pop();

		auto it = std::find_if(
				final_nodes.begin(),
				final_nodes.end(), [&node](auto &item) {
					return item->name == node->name;
				});
		
		if (it == final_nodes.end())
			final_nodes.push_back(node);
		else if ((*it)->version < node->version)
			*it = node;

		for (auto &child : node->dependencies)
			nodes_to_visit.push(child.get());
	}

	uint32_t warn_count = 0;
	std::cout << colors::warning;
	for (auto node : final_nodes) {
		for (std::string &warning : node->warnings) {
			std::cout << '\n' << warning << '\n';
			warn_count++;
		}
	}
	std::cout << tc::reset;

	for (auto node : final_nodes) {
		std::cout << "\nDownloading "
				  << colors::main << node->scope
				  << tc::reset << '/'
				  << colors::main << node->name << ' '
				  << tc::reset << node->version << ":\n";

		std::string branch = util::to_string(node->version);
		fs::path path = std::getenv("VOLT_PATH");
		path = path / "packages" / node->scope / node->name / branch;

		try {
			if (!fs::is_empty(path)) {
				std::cout << "Already installed.\n";
				continue;
			}
		} catch (...) {}
		
		auto &url = manifest_cache[node->get_id()]["git"]
				.get_ref<nl::json::string_t &>();
		std::string cmd = "git -c \"advice.detachedHead=false\" "
				"clone --progress --depth 1 --branch " + branch
				+ ' ' + url + ' ' + path.string();
		
		util::shell(cmd, [](std::string_view data) {
			std::cout << data;
		});
	}

	switch (warn_count) {
	case 0:
		std::cout << colors::success
				  << "\nFinished without warnings.\n" << tc::reset;
		break;
	case 1:
		std::cout << colors::warning
				  << "\nFinished with 1 warning.\n" << tc::reset;
		break;
	default:
		std::cout << colors::warning << "\nFinished with "
				+ std::to_string(warn_count) + " warnings.\n"
				  << tc::reset;
	}
}

}
