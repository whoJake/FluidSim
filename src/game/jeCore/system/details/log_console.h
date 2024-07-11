#pragma once

#include "log_details.h"

namespace sys
{
namespace log
{
namespace details
{

class console_target : public target
{
public:
    console_target();
    ~console_target();

    void output(message* msg) override;
};

} // details
} // log
} // sys