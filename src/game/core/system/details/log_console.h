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
private:
    mtl::fixed_vector<color> m_levelToColor;
};

} // details
} // log
} // sys