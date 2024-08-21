#include "AppStartup.h"

AppStartup::AppStartup()
{ }

AppStartup::~AppStartup()
{ }

int AppStartup::run(int argc, const char* argv[])
{
	// decide if theres a local settings file
	// if so, load window settings from there
	// if not, load with default (fullscreen?)

	m_game = std::make_unique<CrawlerGame>();
	return m_game->run(argc, argv);
}