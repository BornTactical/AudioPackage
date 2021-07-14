#ifndef _AudioPackage_AudioPackage_h_
#define _AudioPackage_AudioPackage_h_
#include <Core/Core.h>
#include <portaudio.h>
#include <pa_linux_alsa.h>

namespace Upp {
    
void PortAudioInit();
void PortAudioTerminate();

template <typename T>
struct Stereo {
    T left  {0.0f};
    T right {0.0f};
    
    String ToString() {
        return Format("Left:%f Right:%f", left, right);
    }
};
 
class HostApiInfo : Moveable<HostApiInfo> {
    const PaHostApiInfo& apiInfo;
    int index {-1};
public:
    
    int             Version()             const { return apiInfo.structVersion; }
    PaHostApiTypeId Type()                const { return apiInfo.type; }
    String          Name()                const { return apiInfo.name; }
    int             DeviceCount()         const { return apiInfo.deviceCount; }
    PaDeviceIndex   DefaultInputDevice()  const { return apiInfo.defaultInputDevice; }
    PaDeviceIndex   DefaultOutputDevice() const { return apiInfo.defaultOutputDevice; }
    
    String ToString() {
        String ret;
        ret << "Version: "                   << Version() << "\n"
            << "Name:    "                   << Name()    << "\n"
            << "Device count "               << DeviceCount() << "\n"
            << "Default Input Device idx: "  << DefaultInputDevice() << "\n"
            << "Default Output Device idx: " << DefaultOutputDevice() << "\n";
            
        return ret;
    }
    
    HostApiInfo(const PaHostApiInfo* info, int index) : apiInfo(*info), index(index) { }
};

class DeviceInfo: Moveable<DeviceInfo> {
    const PaDeviceInfo& deviceInfo;
    int index {-1};
    
public:
    int            Index()             const { return index; }
    String         Name()              const { return deviceInfo.name; }
    int            MaxInputChannels()  const { return deviceInfo.maxInputChannels; }
    int            MaxOutputChannels() const { return deviceInfo.maxOutputChannels; }
    PaHostApiIndex HostApi()           const { return deviceInfo.hostApi; }
    PaTime DefaultLowInputLatency()    const { return deviceInfo.defaultLowInputLatency;   }
    PaTime DefaultLowOutputLatency()   const { return deviceInfo.defaultLowOutputLatency;  }
    PaTime DefaultHighInputLatency()   const { return deviceInfo.defaultHighInputLatency;  }
    PaTime DefaultHighOutputLatency()  const { return deviceInfo.defaultHighOutputLatency; }
    
    String ToString() {
        String ret;
        ret << "Name:                        " << Name()                     << "\n"
            << "Index:                       " << Index()                    << "\n"
            << "Max Input Channels:          " << MaxInputChannels()         << "\n"
            << "Max Output Channels:         " << MaxOutputChannels()        << "\n"
            << "Host Api                     " << PaHostApiIndex()           << "\n"
            << "Default Low Input Latency:   " << DefaultLowInputLatency()   << "\n"
            << "Default Low Output Latency:  " << DefaultLowOutputLatency()  << "\n"
            << "Default High Input Latency:  " << DefaultHighInputLatency()  << "\n"
            << "Default High Output Latency: " << DefaultHighOutputLatency() << "\n";
        
        return ret;
    }
    
    DeviceInfo(const PaDeviceInfo* info, int index) : deviceInfo(*info), index(index) {
    }
};

static int paCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData );

class AudioStream {
    bool  isOpen { false };
    const int defaultOutDevice { 0 };
    const int defaultInDevice  { 0 };
    
    int sampleRate             { 44100 };
    int framesPerBuffer        { 65536 };
    
    PaStream* stream { nullptr };
    PaStreamParameters inputParameters  { 0 };
    PaStreamParameters outputParameters { 0 };
    
public:
    bool IsOpen() const { return isOpen; }
    
    Vector<DeviceInfo>  Devices;
    Vector<HostApiInfo> HostApis;
    
    AudioStream& Device(int devIdx)               { outputParameters.device = devIdx; return *this; }
    AudioStream& ChannelCount(int channels)       { outputParameters.channelCount = channels; return *this; }
    AudioStream& SampleFormat(PaSampleFormat fmt) { outputParameters.sampleFormat = fmt; return *this; }
    AudioStream& Latency(PaTime latency)          { outputParameters.suggestedLatency = latency; return *this; }
    AudioStream& SampleRate(int rate)             { sampleRate = rate; return *this; }
    AudioStream& FramesPerBuffer(int frameCount)  { framesPerBuffer = frameCount; return *this; }
    AudioStream& HostApiStreamInfo(void* apiInfo) { outputParameters.hostApiSpecificStreamInfo = apiInfo; return *this; }
    AudioStream& EnableDolby()                    { PaAlsaStreamInfo streamInfo; return *this; }
    
    Event<void *, unsigned long> WhenRequest;
    PaSampleFormat SampleFormat() const { return outputParameters.sampleFormat; };
    int            ChannelCount() const { return outputParameters.channelCount; };

    bool    IsInterleaved();
    bool    IsVariableFrameSize();
    void    Open2();
    void    Open();
    PaError Abort();
    void    Close();
    bool    IsPlaying();
    PaError Put(void *data, unsigned long size);
    AudioStream();
   ~AudioStream() = default;
};

static int paCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    AudioStream* s = (AudioStream*)userData;
    s->WhenRequest(outputBuffer, framesPerBuffer);
    return paContinue;
}

}

#endif
