#include "PlayerCommands.hpp"

CommandType PlayCommand::GetType(){
    return CommandType::Play;
}

CommandType PauseCommand::GetType(){
    return CommandType::Pause;
}

CommandType ResumeCommand::GetType(){
    return CommandType::Resume;
}

SeekCommand::SeekCommand(unsigned long long pos) : m_seekFramePos{pos} {}

CommandType SeekCommand::GetType(){
    return CommandType::Seek;
}

const constexpr unsigned long long SeekCommand::GetPos() const noexcept{
    return this->m_seekFramePos;
}