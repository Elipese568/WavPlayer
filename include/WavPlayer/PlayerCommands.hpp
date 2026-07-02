#pragma once

#include <variant>

#include "PlayerCommand.hpp"
#include "Typedefs.hpp"

class PlaceholderCommand : public PlayerCommand{
public:
    CommandType GetType() override;
};

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

class ReplayCommand : public PlayerCommand{
public:
    CommandType GetType() override;
};

class SeekCommand : public PlayerCommand{
private:
    AudioFramePos m_seekFramePos;
public:
    SeekCommand(AudioFramePos pos);
    CommandType GetType() override;
    const constexpr AudioFramePos GetPos() const noexcept{
        return this->m_seekFramePos;
    }
};

typedef std::variant<
    PlayCommand,
    PauseCommand,
    ResumeCommand,
    SeekCommand,
    ReplayCommand,
    PlaceholderCommand
> UniPlayerCommand;