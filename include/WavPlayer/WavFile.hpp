#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <chrono>

#include <Windows.h>
#include <mmreg.h>

#include "Utility.hpp"
#include "IAudioStream.hpp"
#include "IAudioSource.hpp"

struct WavChunkHead
{
    char id[4];
    DWORD size;
    char format[4];
};

struct FormatChunkHead
{
    char id[4];
    DWORD size;
};

class WavFile : public IAudioSource
{
private:
    std::filesystem::path m_path;

    std::vector<BYTE> m_formatBuffer;
    WAVEFORMATEX* m_format = nullptr;

    AudioMetadata m_metadata;

    std::streampos m_dataBegin = 0;

    WavFile() = default;
public:
    WavFile(const WavFile&) = delete;
    WavFile& operator=(const WavFile&) = delete;

    WavFile(WavFile&&) = default;
    WavFile& operator=(WavFile&&) = default;

    static WavFile Open(const std::filesystem::path& path);

    const WAVEFORMATEX* Format() const noexcept;
    
    std::unique_ptr<IAudioStream> CreateStream();

    const AudioMetadata& Metadata() const;
};