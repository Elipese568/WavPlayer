#include "Player.hpp"

UINT32 Player::fillBuffer(BYTE** pBuffer, UINT32 bytes)
{
    BYTE* buffer = *pBuffer;

    m_wav.Stream().read(
        address_to(buffer, char),
        bytes
    );

    return static_cast<UINT32>(m_wav.Stream().gcount());
}

void Player::fillEventThreadProc()
{
    DWORD taskIndex = 0;
    HANDLE taskHandle = AvSetMmThreadCharacteristics("Pro Audio", &taskIndex);

    AvSetMmThreadPriority(taskHandle, AVRT_PRIORITY_HIGH);

    auto blockAlign = m_wav.Format()->nBlockAlign;

    while (!m_exitThread)
    {
        int r = WaitForSingleObject(m_fillEvent, INFINITE);

        if (r == WAIT_OBJECT_0 && m_isPlaying)
        {
            UINT32 padding = 0;
            m_client->GetCurrentPadding(&padding);

            UINT32 frames = m_blockSize - padding;
            UINT32 needByteCount = frames * blockAlign;

            BYTE* buffer;
            m_render->GetBuffer(frames, &buffer);

            auto filled = fillBuffer(&buffer, needByteCount);
            m_currentBytePos += filled;

            m_render->ReleaseBuffer(filled / blockAlign, 0);

            if (filled != needByteCount)
            {
                Stop();
            }
        }
    }

    AvRevertMmThreadCharacteristics(taskHandle);
}

Player::Player(ComObject<IAudioClient> client, WavFile&& wav)
    : m_client(std::move(client))
    , m_render(m_client.Service<IAudioRenderClient>())
    , m_wav(std::move(wav))
{
    m_fillEvent = CreateEvent(nullptr, false, false, nullptr);

    m_client->GetBufferSize(&m_blockSize);
    m_client->SetEventHandle(m_fillEvent);

    m_eventThread = std::thread(&Player::fillEventThreadProc, this);
}

Player::~Player()
{
    Stop();

    m_exitThread = true;
    SetEvent(m_fillEvent);

    if (m_eventThread.joinable())
    {
        m_eventThread.join();
    }

    CloseHandle(m_fillEvent);
}

void Player::Play()
{
    m_wav.ResetStream();

    BYTE* buffer;
    m_render->GetBuffer(m_blockSize, &buffer);

    fillBuffer(
        &buffer,
        m_blockSize * m_wav.Format()->nBlockAlign
    );

    m_render->ReleaseBuffer(m_blockSize, 0);

    m_isPlaying = true;
    m_client->Start();
}

void Player::Pause()
{
    m_client->Stop();
    m_isPlaying = false;
}

void Player::Resume()
{
    m_isPlaying = true;
    m_client->Start();
}

void Player::Stop()
{
    m_isPlaying = false;

    m_client->Stop();
    m_client->Reset();

    m_wav.ResetStream();
}

void Player::Replay()
{
    Stop();
    Play();
}

std::chrono::milliseconds Player::GetCurrentProgress() const noexcept{
    return std::chrono::milliseconds(static_cast<long long>(static_cast<double>(m_currentBytePos) * 1000 / m_wav.Format()->nAvgBytesPerSec));
}