#include <iostream>
#include "SandboxApp.h"

int main(int argc, const char* argv[])
{
    {
        SandboxApp app;
        app.run(argc, argv);
    }
    return 0;
}