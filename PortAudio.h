#ifndef _AudioPackage_PortAudio_h_
#define _AudioPackage_PortAudio_h_

#include <Core/Core.h>
#include <portaudio.h>

namespace Upp {
	
void PortAudioInit() {
	auto err = Pa_Initialize();
	if(err != paNoError) throw Exc(Format("Error initializing PortAudio code %d", err));
}

void PortAudioTerminate() {
	Pa_Terminate();
}

INITBLOCK {
	PortAudioInit();
}

EXITBLOCK {
	PortAudioTerminate();
}

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
//		    << "Type:    "                   << Type()    << "\n"
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

Vector<DeviceInfo> GetAudioDevicesInfo() {
	Vector<DeviceInfo>  devices;
	int deviceCount = Pa_GetDeviceCount();
	if(deviceCount < 0) throw Exc(Format("Bad device count: %d devices reported", deviceCount));
	for(int i = 0; i < deviceCount; i++) devices.Create(Pa_GetDeviceInfo(i), i);
	return pick(devices);
}

Vector<HostApiInfo> GetAudioHostApisInfo() {
	Vector<HostApiInfo> hostApis;
	int apiCount = Pa_GetHostApiCount();
	if(apiCount < 0) throw Exc(Format("Bad host API count: %d apis reported", apiCount));
	for(int i = 0; i < apiCount; i++) hostApis.Create(Pa_GetHostApiInfo(i), i);
	return pick(hostApis);
}


enum class SampleFormat : unsigned int {
	Float32 = paFloat32,
	Int32   = paInt32,
	Int24   = paInt24,
	Int16   = paInt16,
	Int8    = paInt8,
	UInt8   = paUInt8,
	Custom  = paCustomFormat,
	NonInterleaved = paNonInterleaved
};

class AudioStream {
	bool  isOpen { false };
	const int defaultOutDevice;
	const int defaultInDevice;
    PaStream* stream { nullptr };
	
public:
	bool IsOpen() const { return isOpen; }
	
	Vector<DeviceInfo>  Devices;
	Vector<HostApiInfo> HostApis;
	
	void Open(SampleFormat fmt = SampleFormat::Float32, int numChannels = 2, int sampleRate=44100, int framesPerBuffer=512) {
		Open(defaultOutDevice, fmt, numChannels, sampleRate, framesPerBuffer);
	}
	
	void Open(int deviceID, SampleFormat fmt = SampleFormat::Float32, int numChannels=2, int sampleRate=44100, int framesPerBuffer=512) {
		auto& device = Devices[deviceID];
		
		Cout() << device.ToString() << "\n";
		
		PaStreamParameters outputParameters {
			deviceID,
			numChannels,
			(PaSampleFormat)fmt,
			device.DefaultLowOutputLatency(),
			nullptr
		};
		
		auto err = Pa_OpenStream(&stream, nullptr, &outputParameters, sampleRate, framesPerBuffer, paClipOff, nullptr, nullptr);
		if( err != paNoError ) throw Exc(Format("Error opening stream! %d", err));
		isOpen = true;
		
        err = Pa_StartStream(stream);
        if( err != paNoError ) throw Exc(Format("Error starting stream! %d", err));
	}
	
	void Close() {
        auto err = Pa_StopStream(stream);
        if( err != paNoError ) throw Exc(Format("Error stopping stream! %d", err));
	}
	
	void Put32(void *data, dword size) {
		auto err = Pa_WriteStream(stream, data, size);
		if( err != paNoError ) throw Exc(Format("Error writing stream! %d", err));
	}
	
	AudioStream() : defaultOutDevice(Pa_GetDefaultOutputDevice()), defaultInDevice(Pa_GetDefaultInputDevice()) {
		int deviceCount = Pa_GetDeviceCount();
		if(deviceCount < 0) throw Exc(Format("Bad device count: %d devices reported", deviceCount));
		for(int i = 0; i < deviceCount; i++) Devices.Create(Pa_GetDeviceInfo(i), i);
		
		int apiCount = Pa_GetHostApiCount();
		if(apiCount < 0) throw Exc(Format("Bad host API count: %d apis reported", apiCount));
		for(int i = 0; i < apiCount; i++) HostApis.Create(Pa_GetHostApiInfo(i), i);
	}
	
};

}
#endif
