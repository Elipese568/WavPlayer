#pragma once
#include <windef.h>
#include <audiopolicy.h>
#include <string>
#include <memory>

#include "IAudioStream.hpp"
#include "AudioMetadata.hpp"

class IAudioSource{
public:
    virtual const WAVEFORMATEX* Format() const = 0;
    virtual std::unique_ptr<IAudioStream> CreateStream() = 0;
    virtual const AudioMetadata& Metadata() const = 0;
};