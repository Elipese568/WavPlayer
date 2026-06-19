#include "Player.hpp"
#include "Utility.hpp"
#include "PlayerCommands.hpp"
#include <functional>
#include <memory>

UINT32 Player::fillBuffer(BYTE* pBuffer, UINT32 bytes)
{
    BYTE* buffer = pBuffer;

    m_wav.Stream().read(
        address_to(buffer, char),
        bytes
    );
    return static_cast<UINT32>(m_wav.Stream().gcount());
}

void Player::fillEventThreadProc(std::stop_token& stopToken)
{
    DWORD taskIndex = 0;
    HANDLE taskHandle = AvSetMmThreadCharacteristics("Pro Audio", &taskIndex);

    AvSetMmThreadPriority(taskHandle, AVRT_PRIORITY_HIGH);

    auto blockAlign = m_wav.Format()->nBlockAlign;

    while (!stopToken.stop_requested())
    {
        UINT32 padding = 0;
        m_client->GetCurrentPadding(&padding);
        switch(m_state.load()) {
            case PlayerState::Commanded: {
                const UniPlayerCommand command = m_command.load();
                std::visit(
                    overloads {
                        [this, blockAlign](const PlayCommand& cmd){
                            // Prefill
                            BYTE* buffer;
                            m_render->GetBuffer(m_blockSize, &buffer);
                            fillBuffer(buffer, m_blockSize * blockAlign);

                            m_render->ReleaseBuffer(m_blockSize, 0);
                            m_currentFramePos += m_blockSize;

                            // Start
                            m_client->Start();
                            RequestState(PlayerState::Playing);
                        },
                        [this, blockAlign](const SeekCommand& seekCmd){
                            auto pos = seekCmd.GetPos();

                            m_client->Stop();
                            m_client->Reset();

                            m_currentFramePos.store(pos);

                            m_wav.ResetStream();
                            m_wav.Stream().seekg(static_cast<std::streamoff>(pos) * blockAlign, std::ios::cur);
                        },
                        [this](const PauseCommand& cmd){
                            
                        }
                    }, command);
            }
            case PlayerState::Playing:{
                int r = WaitForSingleObject(m_fillEvent, 0);
                if (r == WAIT_OBJECT_0)
                {
                    UINT32 frames = m_blockSize - padding;
                    UINT32 needByteCount = frames * blockAlign;

                    BYTE* buffer;
                    m_render->GetBuffer(frames, &buffer);

                    auto filled = fillBuffer(buffer, needByteCount);
                    
                    if (filled < needByteCount)
                    {
                        memset(buffer + filled, 0, needByteCount - filled);
                        m_state = PlayerState::Eof;
                    }
                    m_render->ReleaseBuffer(frames, 0);
                
                    UINT64 curFrameCount = 0;

                    m_currentFramePos = m_currentFramePos + filled / blockAlign;
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

Player::Player(ComObject<IAudioClient> client, bool isExclusiveMode, WavFile&& wav)
    : m_client(std::move(client))
    , m_render(m_client.Service<IAudioRenderClient>())
    , m_clock(m_client.Service<IAudioClock>())
    , m_wav(std::move(wav))
{
    m_fillEvent = CreateEvent(nullptr, false, false, nullptr);

    if(!isExclusiveMode){
        WAVEFORMATEX* mixFormat;

        m_client->GetMixFormat(&mixFormat);
        m_realSampleRate = mixFormat->nSamplesPerSec;

        CoTaskMemFree(mixFormat);
    }
    else {
        m_realSampleRate = m_wav.Format()->nSamplesPerSec;
    }

    m_client->GetBufferSize(&m_blockSize);
    m_client->SetEventHandle(m_fillEvent);

    m_eventThread = std::jthread(
        [this](std::stop_token st)
        {
            this->fillEventThreadProc(st);
        }
    );
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

void Player::RequestState(Player::PlayerState s)
{
    Player::PlayerState old = m_state.exchange(s);
    m_prevState.store(old);
}

void Player::StartPlay()
{
    m_wav.ResetStream();
    m_currentFramePos = 0;

    m_command.store(PlayCommand());
    RequestState(PlayerState::Commanded);
}

void Player::Pause()
{
    m_client->Stop();
    RequestState(PlayerState::Paused);
}

void Player::Resume()
{
    m_client->Start();
    RequestState(PlayerState::Playing);
}

void Player::Stop()
{
    m_client->Stop();
    m_client->Reset();

    m_wav.ResetStream();
    RequestState(PlayerState::Stopped);
}

void Player::Replay()
{
    Stop();
    StartPlay();
}

void Player::Seek(UINT32 framePos){

}


std::chrono::milliseconds Player::GetCurrentProgress() const noexcept{
    return std::chrono::milliseconds(static_cast<long long>(static_cast<double>(this->GetRenderedFrameCount()) / m_wav.GetAudioFrameCount() * m_wav.GetTotalDuration().count()));
}
DWORD Player::GetRenderedFrameCount() const noexcept{
    //return static_cast<DWORD>(static_cast<double>(this->m_currentFramePos) * (static_cast<double>(this->m_wav.Format()->nSamplesPerSec) / this->m_realSampleRate)); // Relative ratio
    return this->m_currentFramePos;
}