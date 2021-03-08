#pragma once

#include <string>
#include <vector>

class command {
public:
	const std::string name, syntax, description;

	virtual ~command() = default;

	virtual void run(const std::vector<std::string> &args) const = 0;
	
protected:
	command(const std::string &name,
			const std::string &syntax,
			const std::string &description);
};
