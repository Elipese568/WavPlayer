#include "PlayerCommands.hpp"

CommandType PlaceholderCommand::GetType(){
    return CommandType::Undefined;
}

CommandType PlayCommand::GetType(){
    return CommandType::Play;
}

CommandType PauseCommand::GetType(){
    return CommandType::Pause;
}

CommandType ResumeCommand::GetType(){
    return CommandType::Resume;
}

CommandType ReplayCommand::GetType(){
    return CommandType::Replay;
}

SeekCommand::SeekCommand(unsigned long long pos) : m_seekFramePos{pos} {}

CommandType SeekCommand::GetType(){
    return CommandType::Seek;
}