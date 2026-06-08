#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>

#include <Windows.h>
#include <mmreg.h>

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

class WavFile
{
private:
    std::ifstream m_stream;

    std::vector<BYTE> m_formatBuffer;
    WAVEFORMATEX* m_format = nullptr;

    std::streampos m_dataBegin = 0;

public:
    WavFile() = default;

    WavFile(const WavFile&) = delete;
    WavFile& operator=(const WavFile&) = delete;

    WavFile(WavFile&&) = default;
    WavFile& operator=(WavFile&&) = default;

    static WavFile Open(const std::filesystem::path& path);

    WAVEFORMATEX* Format() noexcept;

    const WAVEFORMATEX* Format() const noexcept;

    std::istream& Stream() noexcept;

    const std::istream& Stream() const noexcept;

    std::streampos DataBegin() const noexcept;

    void ResetStream();
};