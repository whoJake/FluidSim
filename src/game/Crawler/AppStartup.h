#pragma once

#include "game/CrawlerGame.h"

class AppStartup
{
public:
	AppStartup();
	~AppStartup();

	int run(int argc, const char* argv[]);
private:
	std::unique_ptr<CrawlerGame> m_game;
};