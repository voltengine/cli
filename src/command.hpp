#pragma once

#include "pch.hpp"

class command {
public:
	const std::string name, syntax, description;

	virtual ~command() = default;

	virtual bool run(const std::vector<std::string> &args) const = 0;
	
protected:
	command(std::string_view name,
			std::string_view syntax,
			std::string_view description);
};
