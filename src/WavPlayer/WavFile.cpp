#define NOMINMAX

#include "WavFile.hpp"
#include "Utility.hpp"
#include "IAudioStream.hpp"
#include "Typedefs.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <cmath>

namespace {
    class WavAudioStream : public IAudioStream{
    private:
        std::ifstream m_stream;
        std::streampos m_dataBegin;
        WORD m_blockAlign;
    public:
        WavAudioStream(const std::filesystem::path& path, std::streampos dataBegin, WORD blockAlign)
            : m_stream{path, std::ios::in | std::ios::binary},
              m_dataBegin{dataBegin},
              m_blockAlign{blockAlign} {}
        
        AudioFrameUnit ReadFrames(BYTE* buf, AudioFrameUnit frameCountNeeded){
            UINT32 bytes = frameCountNeeded * m_blockAlign;
            m_stream.read(address_to(buf, char), bytes);
            return static_cast<UINT32>(m_stream.gcount());
        }

        AudioFramePos FramePosition() {
            return static_cast<AudioFramePos>((m_stream.tellg() - m_dataBegin) / m_blockAlign);
        }

        void Seek(UINT32 frame){
            m_stream.seekg(m_dataBegin, std::ios_base::beg);
            m_stream.seekg(frame * m_blockAlign, std::ios_base::cur);
        }

        bool Eof(){
            return m_stream.eof();
        }
    };
}

WavFile WavFile::Open(const std::filesystem::path& path)
{
    WavFile result;
    result.m_path = path;
    result.m_metadata.name = path.filename().string();

    std::ifstream stream{path, std::ios::in | std::ios::binary};

    if(!stream)
        throw std::runtime_error("Cannot open wav file.");

    auto wavHead = readOf<WavChunkHead>(stream);
    // std::cout << "--- Wav Head Data ---"            << '\n'
    //         << "Chunk Id:     " << wavHead.id     << '\n'
    //         << "Chunk Size:   " << wavHead.size   << '\n'
    //         << "Chunk Format: " << wavHead.format << std::endl;

    if(std::memcmp(wavHead.id, "RIFF", 4) != 0)
        throw std::runtime_error("Not a RIFF file.");

    if(std::memcmp(wavHead.format, "WAVE", 4) != 0)
        throw std::runtime_error("Not a WAVE file.");

    char emptyFourCC[4] = {0};
    while(true)
    {
        auto chunk = readOf<FormatChunkHead>(stream);
        // std::cout << "--- Chunk Head Data ---"     << '\n'
        //           << "Chunk ID: "   << chunk.id   << '\n'
        //           << "Chunk Size: " << chunk.size << std::endl;
        if(std::memcmp(chunk.id, "fmt ", 4) == 0)
        {
            result.m_formatBuffer.resize(chunk.size,0);

            stream.read(
                reinterpret_cast<char*>(result.m_formatBuffer.data()),
                chunk.size
            );

            if(!stream)
                throw std::runtime_error("Cannot read fmt chunk.");

            result.m_formatBuffer.resize(std::max(sizeof(WAVEFORMATEX), (decltype(sizeof(WAVEFORMATEX)))chunk.size));

            result.m_format = reinterpret_cast<WAVEFORMATEX*>(result.m_formatBuffer.data());
        }
        else if(std::memcmp(chunk.id, "data", 4) == 0)
        {
            result.m_dataBegin = stream.tellg();
            double seconds =
                static_cast<double>(chunk.size) /
                result.m_format->nAvgBytesPerSec;

            result.m_metadata.totalDuration =
                std::chrono::milliseconds(static_cast<long long>(seconds * 1000.0));
            
            result.m_metadata.audioFrameCount = static_cast<unsigned long>(static_cast<double>(chunk.size) / result.m_format->nBlockAlign);
        }
        else
        {
            stream.seekg(chunk.size, std::ios::cur);
            break;
        }
    }

    if(result.m_format == nullptr)
        throw std::runtime_error("fmt chunk not found.");
    return result;
}

const WAVEFORMATEX* WavFile::Format() const noexcept {
    return m_format;
}
    
std::unique_ptr<IAudioStream> WavFile::CreateStream() {
    return std::make_unique<WavAudioStream>(m_path, m_dataBegin, Format()->nBlockAlign);
}

const AudioMetadata& WavFile::Metadata() const {
    return m_metadata;
}