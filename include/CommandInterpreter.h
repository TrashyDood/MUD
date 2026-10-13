#pragma once

#include <array>
#include <string>

constexpr uint16_t MaxArgsCount = 10;

struct CommandArgs
{
    std::array<std::string, MaxArgsCount> argsBuffer;
    uint16_t argsCount = 0;

    void AddArg(std::string arg);
};

class CommandInterpreter
{
    
};