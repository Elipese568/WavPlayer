#pragma once

#include <variant>

#include "PlayerCommand.hpp"

class PlayCommand : public PlayerCommand{
public:
    CommandType GetType() override;
};

class PauseCommand : public PlayerCommand{
public:
    CommandType GetType() override;
};

class ResumeCommand : public PlayerCommand{
public:
    CommandType GetType() override;
};


class SeekCommand : public PlayerCommand{
private:
    unsigned long long m_seekFramePos;
public:
    SeekCommand(unsigned long long pos);
    CommandType GetType() override;
    const constexpr unsigned long long GetPos() const noexcept;
};

using UniPlayerCommand = std::variant<
    PlayCommand,
    PauseCommand,
    ResumeCommand,
    SeekCommand
>;