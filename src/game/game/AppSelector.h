#pragma once

#include "base/app.h"

class AppSelector
{
public:
	AppSelector() = default;
	~AppSelector() = default;

	int run(int argc, const char* argv[]);
private:
	std::unique_ptr<fw::app> m_app;
};