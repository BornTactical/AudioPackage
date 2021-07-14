#include <AudioPackage/AudioPackage.h>

namespace Upp {
	bool AudioStream::IsInterleaved() {
		return !(outputParameters.sampleFormat & paNonInterleaved);
	}
	
	bool AudioStream::IsVariableFrameSize() {
		return (framesPerBuffer == 0);
	}
	
	void AudioStream::Open2() {
		auto err = Pa_OpenStream(&stream, nullptr, &outputParameters, sampleRate, framesPerBuffer, paClipOff, paCallback, (void*)this);
		if( err != paNoError ) throw Exc(Format("Error opening stream! %d", err));
		isOpen = true;
		
        err = Pa_StartStream(stream);
        if( err != paNoError ) throw Exc(Format("Error starting stream! %d", err));
	}
	
	void AudioStream::Open() {
		auto err = Pa_OpenStream(&stream, nullptr, &outputParameters, sampleRate, framesPerBuffer, paClipOff, nullptr, nullptr);
		if( err != paNoError ) throw Exc(Format("Error opening stream! %d", err));
		isOpen = true;
		
        err = Pa_StartStream(stream);
        if( err != paNoError ) throw Exc(Format("Error starting stream! %d", err));
	}
	
	PaError AudioStream::Abort() {
		return Pa_AbortStream(stream);
	}
	
	void AudioStream::Close() {
        auto err = Pa_StopStream(stream);
        if( err != paNoError ) throw Exc(Format("Error stopping stream! %d", err));
	}
	
	bool AudioStream::IsPlaying() {
		return Pa_IsStreamActive(stream);
	}
	
	PaError AudioStream::Put(void *data, unsigned long size) {
		if(!IsOpen()) throw Exc("Stream not open!");
		PaError err = Pa_WriteStream(stream, data, size);
		if(err) {
			LOG(Pa_GetErrorText(err));
		}
		
		return err;
	}
	
	AudioStream::AudioStream()  : defaultOutDevice(Pa_GetDefaultOutputDevice()), defaultInDevice(Pa_GetDefaultInputDevice()) {
		int deviceCount = Pa_GetDeviceCount();
		if(deviceCount < 0) throw Exc(Format("Bad device count: %d devices reported", deviceCount));
		for(int i = 0; i < deviceCount; i++) Devices.Create(Pa_GetDeviceInfo(i), i);
		
		int apiCount = Pa_GetHostApiCount();
		if(apiCount < 0) throw Exc(Format("Bad host API count: %d apis reported", apiCount));
		for(int i = 0; i < apiCount; i++) HostApis.Create(Pa_GetHostApiInfo(i), i);
		
		Device(defaultOutDevice)
			.ChannelCount(2)
			.SampleFormat(paFloat32)
			.Latency(Devices[defaultOutDevice].DefaultHighOutputLatency())
			.HostApiStreamInfo(nullptr);
	}
}
