#define NOMINMAX

#include "WavFile.hpp"
#include "Utility.hpp"

#include <cstring>
#include <filesystem>
#include <cmath>

WavFile WavFile::Open(const std::filesystem::path& path)
{
    WavFile result;

    result.m_stream.open(path, std::ios::in | std::ios::binary);

    if(!result.m_stream)
        throw std::runtime_error("Cannot open wav file.");

    auto wavHead = readOf<WavChunkHead>(result.m_stream);
    std::cout << "--- Wav Head Data ---"            << '\n'
            << "Chunk Id:     " << wavHead.id     << '\n'
            << "Chunk Size:   " << wavHead.size   << '\n'
            << "Chunk Format: " << wavHead.format << std::endl;

    if(std::memcmp(wavHead.id, "RIFF", 4) != 0)
        throw std::runtime_error("Not a RIFF file.");

    if(std::memcmp(wavHead.format, "WAVE", 4) != 0)
        throw std::runtime_error("Not a WAVE file.");

    while(true)
    {
        auto chunk = readOf<FormatChunkHead>(result.m_stream);
        std::cout << "--- Chunk Head Data ---"     << '\n'
                  << "Chunk ID: "   << chunk.id   << '\n'
                  << "Chunk Size: " << chunk.size << std::endl;
        if(std::memcmp(chunk.id, "fmt ", 4) == 0)
        {
            result.m_formatBuffer.resize(chunk.size,0);

            result.m_stream.read(
                reinterpret_cast<char*>(result.m_formatBuffer.data()),
                chunk.size
            );

            if(!result.m_stream)
                throw std::runtime_error("Cannot read fmt chunk.");

            result.m_formatBuffer.resize(std::max(sizeof(WAVEFORMATEX), (decltype(sizeof(WAVEFORMATEX)))chunk.size));

            result.m_format = reinterpret_cast<WAVEFORMATEX*>(result.m_formatBuffer.data());
        }
        else if(std::memcmp(chunk.id, "data", 4) == 0)
        {
            result.m_dataBegin = result.m_stream.tellg();
            break;
        }
        else
        {
            result.m_stream.seekg(chunk.size, std::ios::cur);
        }
    }

    if(result.m_format == nullptr)
        throw std::runtime_error("fmt chunk not found.");

    return result;
}

WAVEFORMATEX* WavFile::Format() noexcept
{
    return m_format;
}

const WAVEFORMATEX* WavFile::Format() const noexcept
{
    return m_format;
}

std::istream& WavFile::Stream() noexcept
{
    return m_stream;
}

const std::istream& WavFile::Stream() const noexcept
{
    return m_stream;
}

std::streampos WavFile::DataBegin() const noexcept
{
    return m_dataBegin;
}

void WavFile::ResetStream()
{
    m_stream.clear();
    m_stream.seekg(m_dataBegin);
}