#pragma once

#include "application/WindowedApplication.h"
#include "platform/events/Event.h"

class GameApp : WindowedApplication
{
public:
    void on_app_startup() override;

    void update() override;

    void on_event(Event& e) override;
private:
};