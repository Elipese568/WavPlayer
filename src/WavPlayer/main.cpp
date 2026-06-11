#define NOMINMAX

#include <typeinfo>
#include <limits>
#include <utility>
#include <optional>

#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <Audioclient.h>

#include <ShlObj.h>

#include <filesystem>

#include <sstream>

#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <thread>

#include <avrt.h>

#include "ComUtility.hpp"
#include "Player.hpp"
#include "WavFile.hpp"
#include "Utility.hpp"
#include "PlayerControllerGui.hpp"

using ::ComUtility::ComObject;

DEFINE_TRAIT(IMMDeviceEnumerator, MMDeviceEnumerator);

constexpr auto streamMaxSize = std::numeric_limits<std::streamsize>::max();

HWND GetProgramHWND(){
    return GetConsoleWindow();
}

std::optional<std::filesystem::path> GetFileConsole(){
    std::cout << "Input wav path: ";
    std::string wavPath;
    std::cin >> wavPath;
    std::cin.ignore(streamMaxSize, '\n');
    if(!std::filesystem::exists(wavPath))
        return std::nullopt;
    return wavPath;
}

DEFINE_TRAIT(IFileOpenDialog, FileOpenDialog);
std::optional<std::filesystem::path> GetFilePicker(){
    auto dialog = ComUtility::Create<IFileOpenDialog>();

    COMDLG_FILTERSPEC rgSpec[] =
    { 
        { L"Wav file", L"*.wav" }
    };
    dialog->SetFileTypes(1, rgSpec);

    auto consoleWindowHandle = GetProgramHWND();
    auto hr = dialog->Show(consoleWindowHandle);
    
    if(hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)){
        return std::nullopt;
    }

    ComObject<IShellItem> item;
    dialog->GetResult(item.Put());

    LPWSTR path = nullptr;
    item->GetDisplayName(SIGDN_FILESYSPATH, &path);
    std::filesystem::path result{path};

    CoTaskMemFree(path);

    return result;
}

struct StartupArguments{
    bool useGui;
    HINSTANCE processInstance;
    HINSTANCE previousProcessInstance;
    int isCmdShow;
};

nocopy(StartupArguments) parseArguments(    
    nocopy(HINSTANCE) hInstance,
    nocopy(HINSTANCE) hPrevInstance,
    nocopy(LPSTR)     cmdline,
    int               nCmdShow
){
    std::stringstream ss(cmdline);
    StartupArguments result{};
    while(!ss.eof()){
        std::string arg;
        ss >> arg;
        if(arg == "--gui" || arg == "-g") result.useGui = true;
    }
    return result;
}

int APIENTRY CliMain(
    const StartupArguments& args
) {
    AllocConsole();
    
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONIN$", "r", stdin);

    auto deviceEnumetator = ComUtility::Create<IMMDeviceEnumerator>();
    ComObject<IMMDevice> device;

    deviceEnumetator->GetDefaultAudioEndpoint(eRender, eConsole, device.Put());
    
    ComObject<IAudioClient> client = device.Activate<IAudioClient>();
    
    auto wavPath = GetFilePicker();
    if(!wavPath.has_value()){
        std::cout << "Selection was cancelled or an error occurred." << std::endl;
        std::cin.get();
        return 0;
    }
    WavFile file = WavFile::Open(wavPath.value());
    
    auto format = file.Format();
    std::cout << "--- Wav Format Data ---"                      << '\n'
              << "Format Tag:      " << format->wFormatTag      << '\n'
              << "Channels:        " << format->nChannels       << '\n'
              << "Samples/Sec:     " << format->nSamplesPerSec  << '\n'
              << "Avg Bytes/Sec:   " << format->nAvgBytesPerSec << '\n'
              << "Block Align:     " << format->nBlockAlign     << '\n'
              << "Bits/Sample:     " << format->wBitsPerSample  << '\n'
              << "Extra Info Size: " << format->cbSize          << std::endl;

    START_COM_GUARD

    ComUtility::CheckHR(client->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |  AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
        0, 0, format, nullptr 
    ));

    Player player{std::move(client), std::move(file)};

    while(1){
        std::cout << "Input Command(/h for help): ";
        std::string cmd;
        std::cin >> cmd;
        if(cmd == "quit") break;
        else if(cmd == "/h"){
            std::cout << "Available commands:\n"
                      << "  play    - Start audio playback\n"
                      << "  pause   - Pause playback\n"
                      << "  resume  - Resume playback\n"
                      << "  stop    - Stop playback\n"
                      << "  replay  - Restart playback from the beginning\n"
                      << "  cur     - Get current position of playback stream\n"
                      << "  quit    - Exit the program\n"
                      << "  /h      - Show this help message\n"
                      << std::endl;
        }
        else if(cmd == "stop") player.Stop();
        else if(cmd == "play") player.Play();
        else if(cmd == "pause") player.Pause();
        else if(cmd == "resume") player.Resume();
        else if(cmd == "replay") player.Replay();
        else if(cmd == "cur"){
            std::cout << player.GetCurrentProgress() << std::endl;
        }
    }

    FAIL_COM_GUARD(r)
        std::cout << "Error: " << r << std::endl;
    END_COM_GUARD

    std::cin.get();
    return 0;
}
int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     cmd,
    int       nCmdShow
) {
    auto comGuard = ComUtility::ComInitializeGuard();

    StartupArguments sa = parseArguments(hInstance, hPrevInstance, cmd, nCmdShow);
    int exitCode;

    if(sa.useGui){
        //exitCode = GuiMain(sa); preversed
    }
    else{
        exitCode = CliMain(sa);
    }

    return exitCode;
}
