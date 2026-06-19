#pragma once

#include <Windows.h>
#include <Audioclient.h>
#include <avrt.h>

#include <thread>
#include <chrono>
#include <atomic>
#include <variant>

#include "ComObject.hpp"
#include "WavFile.hpp"
#include "PlayerCommand.hpp"
#include "PlayerCommands.hpp"
#include "Utility.hpp"

using ::ComUtility::ComObject;

class Player
{
private:
    enum class PlayerState {
        Stopped,
        Playing,
        Paused,
        Eof,
        Commanded
    };

    ComObject<IAudioClient> m_client;
    ComObject<IAudioRenderClient> m_render;
    ComObject<IAudioClock> m_clock;

    WavFile m_wav;
    bool m_eof;

    std::jthread m_eventThread;
    HANDLE m_fillEvent{nullptr};

    std::atomic<PlayerState> m_state{PlayerState::Stopped};
    std::atomic<PlayerState> m_prevState{PlayerState::Stopped};

    std::atomic<UINT64> m_currentFramePos{0};

    std::atomic<std::shared_ptr<UniPlayerCommand>> m_command;

    UINT32 m_blockSize{0};
    DWORD m_realSampleRate;
private:
    UINT32 fillBuffer(BYTE* pBuffer, UINT32 bytes);
    void fillEventThreadProc(std::stop_token&);
    void RequestState(PlayerState state, bool forced);
    void RevertState();
    void SetCommand(UniPlayerCommand cmd);
public:
    Player(ComObject<IAudioClient> client, bool isExclusiveMode, WavFile&& wav);
    ~Player();

    void StartPlay();
    void Pause();
    void Resume();
    void Stop();
    void Replay();
    void Seek(UINT32 framePos);

    std::chrono::milliseconds GetCurrentProgress() const noexcept;
    DWORD GetRenderedFrameCount() const noexcept;
};