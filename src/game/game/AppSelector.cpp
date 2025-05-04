#include "AppSelector.h"
#include "game/CrawlerGame.h"
#include "game/FluidApp.h"

int AppSelector::run(int argc, const char* argv[])
{
	// decide if theres a local settings file
	// if so, load window settings from there
	// if not, load with default (fullscreen?)

	// m_app = std::make_unique<CrawlerGame>();
	m_app = std::make_unique<FluidApp>();
	return m_app->run(argc, argv);
}