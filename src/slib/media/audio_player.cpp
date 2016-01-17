#include "../../../inc/slib/media/audio_player.h"
#include "../../../inc/slib/media/audio_format.h"

SLIB_MEDIA_NAMESPACE_BEGIN

AudioPlayerInfo::AudioPlayerInfo()
{
}

AudioPlayerBufferParam::AudioPlayerBufferParam()
{
	samplesPerSecond = 16000;
	channelsCount = 1;
	frameLengthInMilliseconds = 50;
	bufferLengthInMilliseconds = 1000;
	
	flagAutoStart = sl_true;
}

AudioPlayerParam::AudioPlayerParam()
{
}

AudioPlayerBuffer::AudioPlayerBuffer()
{
	m_lastSample = 0;
}

void AudioPlayerBuffer::write(const AudioData& audioIn)
{
	AudioFormat format;
	sl_uint32 nChannels = m_nChannels;
	if (nChannels == 1) {
		format = audioFormat_Int16_Mono;
	} else {
		format = audioFormat_Int16_Stereo;
	}
	if (audioIn.format == format && (((sl_size)(audioIn.data)) & 1) == 0) {
		m_queue.add((sl_int16*)(audioIn.data), nChannels * audioIn.count);
	} else {
		sl_int16 samples[2048];
		AudioData temp;
		temp.format = format;
		temp.data = samples;
		temp.count = 1024;
		sl_size n = audioIn.count;
		while (n > 0) {
			sl_size m = n;
			if (m > 1024) {
				m = 1024;
			}
			n -= m;
			temp.copySamplesFrom(audioIn, m);
			m_queue.add(samples, nChannels*m);
		}
	}
}

Array<sl_int16> AudioPlayerBuffer::_getProcessData(sl_size count)
{
	Array<sl_int16> data = m_processData;
	if (data.count() >= count) {
		return data;
	} else {
		data = Array<sl_int16>::create(count);
		m_processData = data;
		return data;
	}
}

void AudioPlayerBuffer::_processFrame(sl_int16* s, sl_size count)
{
	if (m_event.isNotNull()) {
		m_event->set();
	}
	PtrLocker<IAudioPlayerBufferListener> listener(m_listener);
	if (listener.isNotNull()) {
		listener->onPlayAudio(this, count / m_nChannels);
	}
	if (!(m_queue.get(s, count))) {
		for (sl_size i = 0; i < count; i++) {
			s[i] = m_lastSample;
		}
	}
	m_lastSample = s[count - 1];
}

SLIB_MEDIA_NAMESPACE_END

