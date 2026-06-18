#pragma once

#include <Windows.h>
#include <Audioclient.h>
#include <avrt.h>

#include <thread>
#include <chrono>
#include <atomic>

#include "ComObject.hpp"
#include "WavFile.hpp"
#include "Utility.hpp"

using ::ComUtility::ComObject;

class Player
{
private:
    ComObject<IAudioClient> m_client;
    ComObject<IAudioRenderClient> m_render;
    ComObject<IAudioClock> m_clock;

    WavFile m_wav;
    bool m_eof;

    std::thread m_eventThread;
    HANDLE m_fillEvent{nullptr};

    std::atomic_bool m_isPlaying{false};
    std::atomic_bool m_exitThread{false};
    std::atomic<UINT64> m_currentFramePos{0};

    UINT32 m_blockSize{0};
    DWORD m_realSampleRate;
private:
    UINT32 fillBuffer(BYTE* pBuffer, UINT32 bytes);
    void fillEventThreadProc();

public:
    Player(ComObject<IAudioClient> client, bool isExclusiveMode, WavFile&& wav);
    ~Player();

    void Play();
    void Pause();
    void Resume();
    void Stop();
    void Replay();

    std::chrono::milliseconds GetCurrentProgress() const noexcept;
    DWORD GetRenderedFrameCount() const noexcept;
};