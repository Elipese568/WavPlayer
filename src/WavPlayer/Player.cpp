#include "Player.hpp"
#include "Utility.hpp"
#include "PlayerCommands.hpp"
#include <functional>
#include <memory>

UINT32 Player::fillBuffer(BYTE* pBuffer, UINT32 frames)
{
    return m_sourceStream->ReadFrames(pBuffer, frames);
}

void Player::fillEventThreadProc(std::stop_token& stopToken)
{
    DWORD taskIndex = 0;
    HANDLE taskHandle = AvSetMmThreadCharacteristics("Pro Audio", &taskIndex);

    AvSetMmThreadPriority(taskHandle, AVRT_PRIORITY_HIGH);

    auto blockAlign = m_source->Format()->nBlockAlign;

    while (!stopToken.stop_requested())
    {
        UINT32 padding = 0;
        m_client->GetCurrentPadding(&padding);
        switch(m_state.load()) {
            case PlayerState::Commanded: {
                auto command = m_command.load();
                std::visit(
                    overloads {
                        [this, blockAlign](const PlayCommand& cmd){
                            // Prefill
                            BYTE* buffer;
                            m_render->GetBuffer(m_blockSize, &buffer);
                            auto filled = fillBuffer(buffer, m_blockSize * blockAlign);

                            m_render->ReleaseBuffer(m_blockSize, 0);

                            // Start
                            m_client->Start();
                            RequestState(PlayerState::Playing, true);
                        },
                        [this, blockAlign](const SeekCommand& seekCmd){
                            auto pos = seekCmd.GetPos();

                            m_client->Stop();
                            m_client->Reset();
                            
                            if(m_prevState.load() == PlayerState::Playing){
                                m_client->Start();
                            }
                        },
                        [this](const PauseCommand& cmd){
                            m_client->Stop();
                            RequestState(PlayerState::Paused, true);
                        },
                        [this](const ResumeCommand& cmd){
                            m_client->Start();
                            RequestState(PlayerState::Playing, true);
                        },
                        [this](const ReplayCommand& cmd){
                            m_client->Stop();
                            m_client->Reset();

                            m_sourceStream->Seek(0);

                            m_command = std::make_shared<UniPlayerCommand>(PlayCommand());
                            RequestState(PlayerState::Commanded, true);
                        },
                        [this](const PlaceholderCommand& cmd){
                            // placeholder
                        }
                    }, *command);
                RevertState();
                break;
            }
            case PlayerState::Playing:{
                int r = WaitForSingleObject(m_fillEvent, 0);
                if (r == WAIT_OBJECT_0)
                {
                    UINT32 frames = m_blockSize - padding;

                    BYTE* buffer;
                    m_render->GetBuffer(frames, &buffer);

                    auto filled = fillBuffer(buffer, frames);
                    
                    if (filled < frames)
                    {
                        memset(buffer + filled * blockAlign, 0, (frames - filled) * blockAlign);
                        m_state = PlayerState::Eof;
                    }
                    m_render->ReleaseBuffer(frames, 0);
                
                    UINT64 curFrameCount = 0;
                }
                break;
            }
            case PlayerState::Eof: {
                if(padding == 0){
                    m_state = PlayerState::Stopped;
                }
                break;
            }
        }
    }

    AvRevertMmThreadCharacteristics(taskHandle);
}

Player::Player(ComObject<IAudioClient> client, bool isExclusiveMode)
    : m_client(std::move(client))
    , m_render(m_client.Service<IAudioRenderClient>())
{
    m_fillEvent = CreateEvent(nullptr, false, false, nullptr);

    if(!isExclusiveMode){
        WAVEFORMATEX* mixFormat;

        m_client->GetMixFormat(&mixFormat);
        m_realSampleRate = mixFormat->nSamplesPerSec;

        CoTaskMemFree(mixFormat);
    }

    m_client->GetBufferSize(&m_blockSize);
    m_client->SetEventHandle(m_fillEvent);

    m_eventThread = std::jthread([this](std::stop_token st){this->fillEventThreadProc(st);});
}

Player::~Player()
{
    Stop();
    
    m_eventThread.request_stop();
    SetEvent(m_fillEvent);

    if (m_eventThread.joinable())
        m_eventThread.join();

    CloseHandle(m_fillEvent);
}

void Player::RequestState(Player::PlayerState s, bool forced)
{
    Player::PlayerState old = m_state.exchange(s);
    m_prevState.store(forced? s : old);
}

void Player::RevertState(){
    m_state.store(m_prevState);
}


void Player::SetCommand(UniPlayerCommand cmd){
    m_command = std::make_shared<UniPlayerCommand>(cmd);
    RequestState(PlayerState::Commanded, false);
}

void Player::SetSource(std::shared_ptr<IAudioSource> source){
    m_source = std::move(source);
    m_sourceStream = m_source->CreateStream();
}

std::shared_ptr<IAudioSource> Player::GetSource() const {
    return m_source;
}

void Player::StartPlay()
{
    m_sourceStream->Seek(0);
    SetCommand(PlayCommand());
}

void Player::Pause()
{
    SetCommand(PauseCommand());
}

void Player::Resume()
{
    SetCommand(ResumeCommand());
}

void Player::Stop()
{
    m_client->Stop();
    m_client->Reset();
    RequestState(PlayerState::Stopped, false);
}

void Player::Replay()
{
    SetCommand(ReplayCommand());
}

void Player::Seek(AudioFramePos framePos){
    SetCommand(SeekCommand(framePos));
}

std::chrono::milliseconds Player::GetCurrentProgress() const noexcept{
    return std::chrono::milliseconds(static_cast<long long>(static_cast<double>(this->GetRenderedFrameCount()) / m_source->Metadata().audioFrameCount * m_source->Metadata().totalDuration.count()));
}
AudioFrameUnit Player::GetRenderedFrameCount() const noexcept{
    //return static_cast<DWORD>(static_cast<double>(this->m_currentFramePos) * (static_cast<double>(this->m_wav.Format()->nSamplesPerSec) / this->m_realSampleRate)); // Relative ratio
    return m_sourceStream->FramePosition();
}