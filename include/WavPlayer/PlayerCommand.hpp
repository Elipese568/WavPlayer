#pragma once
#include "CommandType.hpp"

class PlayerCommand{
public:
    virtual CommandType GetType() = 0;
};