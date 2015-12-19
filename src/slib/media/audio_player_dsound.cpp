#include "../../../inc/slib/media/dsound.h"

#if defined(SLIB_PLATFORM_IS_WIN32)

#include <initguid.h>
#include <Windows.h>
#include <MMSystem.h>
#include <dsound.h>

#include "../../../inc/slib/core/platform_windows.h"
#include "../../../inc/slib/core/log.h"
#include "../../../inc/slib/core/thread.h"

#pragma comment(lib, "dsound.lib")

SLIB_MEDIA_NAMESPACE_BEGIN

#define NUM_PLAY_NOTIFICATIONS  2
class _DirectSound_AudioPlayerBuffer : public AudioPlayerBuffer
{
public:
	LPDIRECTSOUND		m_ds;
	LPDIRECTSOUNDBUFFER m_dsBuffer;
	LPDIRECTSOUNDNOTIFY m_dsNotify;
	HANDLE              m_hNotificationEvents[2];

	sl_uint32 m_nSamplesFrame;
	sl_uint32 m_nSamplesBuffer;
	sl_uint32 m_nBufferSize;
	sl_uint32 m_nOffsetNextWrite;
	sl_uint32 m_nNotifySize;

	Ref<Thread> m_thread;
	sl_bool m_flagRunning;
	sl_bool m_flagOpened;

	_DirectSound_AudioPlayerBuffer()
	{
		m_nOffsetNextWrite = m_nNotifySize = 0;
		m_ds = NULL;
		m_dsBuffer = NULL;
		m_dsNotify = NULL;
		m_flagOpened = sl_true;
		m_flagRunning = sl_false;
		m_thread.setNull();
	}

	~_DirectSound_AudioPlayerBuffer()
	{
		release();
	}

	static void logError(String text)
	{
		SLIB_LOG_ERROR("AudioPlayer", text);
	}

	static Ref<_DirectSound_AudioPlayerBuffer> create(const AudioPlayerBufferParam& param, const GUID& deviceID)
	{
		::CoInitializeEx(NULL, COINIT_MULTITHREADED);

		LPDIRECTSOUND ds;
		HRESULT hr = DirectSoundCreate(&deviceID, &ds, NULL);
		if (SUCCEEDED(hr)) {
			hr = ds->SetCooperativeLevel(::GetDesktopWindow(), DSSCL_NORMAL);
			if (SUCCEEDED(hr)) {
				WAVEFORMATEX wf;
				wf.wFormatTag = WAVE_FORMAT_PCM;
				wf.nChannels = 1;
				wf.wBitsPerSample = 16;
				wf.nSamplesPerSec = param.samplesPerSecond;
				wf.nBlockAlign = wf.wBitsPerSample * wf.nChannels / 8;
				wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
				wf.cbSize = 0;

				sl_uint32 samplesPerFrame = wf.nSamplesPerSec * param.frameLengthInMilliseconds / 1000;
				sl_uint32 sizeBuffer = samplesPerFrame * wf.nBlockAlign * 3;
				sl_int32 notifySize = sizeBuffer / NUM_PLAY_NOTIFICATIONS;

				DSBUFFERDESC desc;
				ZeroMemory(&desc, sizeof(DSBUFFERDESC));
				desc.dwSize = sizeof(DSBUFFERDESC);
				desc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
				desc.dwBufferBytes = sizeBuffer;
				desc.lpwfxFormat = &wf;

				LPDIRECTSOUNDBUFFER dsBuffer;
				HANDLE hNotificationEvents[2];
				hNotificationEvents[0] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
				hNotificationEvents[1] = ::CreateEventW(NULL, FALSE, FALSE, NULL);

				hr = ds->CreateSoundBuffer(&desc, &dsBuffer, NULL);
				if (SUCCEEDED(hr)) {
					LPDIRECTSOUNDNOTIFY dsNotify;
					hr = dsBuffer->QueryInterface(IID_IDirectSoundNotify, (VOID**)&dsNotify);
					if (SUCCEEDED(hr)) {
						DSBPOSITIONNOTIFY   dsPosNotify[NUM_PLAY_NOTIFICATIONS + 1];
						sl_int32 notifyIndex = 0;

						for (notifyIndex = 0; notifyIndex < NUM_PLAY_NOTIFICATIONS; notifyIndex++) {
							dsPosNotify[notifyIndex].dwOffset = (notifySize * notifyIndex) + notifySize - 1;
							dsPosNotify[notifyIndex].hEventNotify = hNotificationEvents[0];
						}
						dsPosNotify[notifyIndex].dwOffset = DSBPN_OFFSETSTOP;
						dsPosNotify[notifyIndex].hEventNotify = hNotificationEvents[1];

						hr = dsNotify->SetNotificationPositions(NUM_PLAY_NOTIFICATIONS + 1, dsPosNotify);
						if (SUCCEEDED(hr)) {
							hr = dsBuffer->Play(0, 0, DSBPLAY_LOOPING);
							if (SUCCEEDED(hr)) {
								Ref<_DirectSound_AudioPlayerBuffer> ret = new _DirectSound_AudioPlayerBuffer();
								if (ret.isNotNull()) {
									ret->m_ds = ds;
									ret->m_dsBuffer = dsBuffer;
									ret->m_dsNotify = dsNotify;
									ret->m_nNotifySize = notifySize;
									ret->m_hNotificationEvents[0] = hNotificationEvents[0];
									ret->m_hNotificationEvents[1] = hNotificationEvents[1];
									ret->m_nSamplesFrame = samplesPerFrame;
									ret->m_nBufferSize = sizeBuffer;
									ret->m_nSamplesBuffer = param.samplesPerSecond * param.bufferLengthInMilliseconds / 1000;
									ret->m_queue.setQueueSize(ret->m_nSamplesBuffer);
									ret->setListener(param.listener);
									if (param.flagAutoStart) {
										ret->start();
									}
									return ret;
								}
							} else {
								logError("Failed to start direct sound looping");
							}
						} else {
							logError("Failed to set dsound notify positions");
						}
					} else {
						logError("Failed to get dsound notify");
					}
					dsBuffer->Release();
				} else {
					logError("Failed to create dsound buffer 8");
				}
				ds->Release();
				CloseHandle(hNotificationEvents[0]);
				CloseHandle(hNotificationEvents[1]);
			} else {
				logError("Direct sound set CooperativeLevel failed.");
			}
		} else {
			if (hr == DSERR_ALLOCATED) {
				logError("Direct sound playback device is already used");
			} else {
				logError("Can not create direct sound playback device");
			}
		}
		return Ref<_DirectSound_AudioPlayerBuffer>::null();
	}

	void release()
	{
		MutexLocker lock(getLocker());
		if (!m_flagOpened) {
			return;
		}
		stop();
		m_flagOpened = sl_false;
		m_dsBuffer->Release();
		m_dsBuffer = NULL;

		m_dsNotify->Release();
		m_dsNotify = NULL;

		m_ds->Release();
		m_ds = NULL;

		for (int i = 0; i < 2; i++) {
			CloseHandle(m_hNotificationEvents[i]);
		}
	}

	void start()
	{
		MutexLocker lock(getLocker());
		if (!m_flagOpened) {
			return;
		}
		if (m_flagRunning) {
			return;
		}
		m_thread = Thread::start(SLIB_CALLBACK_CLASS(_DirectSound_AudioPlayerBuffer, run, this));
		if (m_thread.isNotNull()) {
			m_flagRunning = sl_true;
		}
	}

	void stop()
	{
		MutexLocker lock(getLocker());
		if (!m_flagOpened) {
			return;
		}
		if (!m_flagRunning) {
			return;
		}
		m_flagRunning = sl_false;
		m_thread->finish();
		::SetEvent(m_hNotificationEvents[1]);
		m_thread->finishAndWait();
		m_thread.setNull();
	}

	void run()
	{
		::CoInitializeEx(NULL, COINIT_MULTITHREADED);
		while (Thread::isNotStoppingCurrent()) {
			DWORD dwWait = ::WaitForMultipleObjects(2, m_hNotificationEvents, FALSE, INFINITE);
			if (dwWait == WAIT_OBJECT_0) {
				onFrame();
			} else {
				m_dsBuffer->Stop();
			}
		}
	}

	void onFrame()
	{
		VOID* pbBuffer = NULL;
		DWORD bufferSize;

		HRESULT hr = m_dsBuffer->Lock(m_nOffsetNextWrite, m_nNotifySize, &pbBuffer, &bufferSize, NULL, NULL, 0L);
		if (SUCCEEDED(hr)) {
			sl_uint32 nSamples = bufferSize / 2;
			sl_int16* s = (sl_int16*)pbBuffer;
			
			_processFrame_S16(s, nSamples);
			
			m_nOffsetNextWrite += bufferSize;
			m_nOffsetNextWrite %= m_nBufferSize;
			m_dsBuffer->Unlock(pbBuffer, bufferSize, NULL, NULL);
		}
	}

	sl_bool isOpened()
	{
		return m_flagOpened;
	}

	sl_bool isRunning()
	{
		return m_flagRunning;
	}
};

class _DirectSound_AudioPlayer : public AudioPlayer
{
public:
	GUID m_deviceID;

	_DirectSound_AudioPlayer()
	{
	}

	~_DirectSound_AudioPlayer()
	{
	}

	static void logError(String text)
	{
		SLIB_LOG_ERROR("AudioPlayer", text);
	}

	static Ref<_DirectSound_AudioPlayer> create(const AudioPlayerParam& param)
	{
		Ref<_DirectSound_AudioPlayer> ret = new _DirectSound_AudioPlayer();
		GUID gid;
		String deviceID = param.deviceId;
		if (deviceID.isEmpty()) {
			gid = DSDEVID_DefaultPlayback;
		} else {
			ListLocker<DeviceProperty> props(queryDeviceInfos());
			sl_size i = 0;
			for (; i < props.count(); i++) {
				if (deviceID == props[i].szGuid) {
					gid = props[i].guid;
					break;
				}
			}
			if (i == props.count()) {
				logError("Failed to find capture device");
			}
		}
		ret->m_deviceID = gid;
		return ret;
	}

	Ref<AudioPlayerBuffer> createBuffer(const AudioPlayerBufferParam& param)
	{
		return Ref<AudioPlayerBuffer>::from(_DirectSound_AudioPlayerBuffer::create(param, m_deviceID));
	}

	struct DeviceProperty {
		GUID guid;
		String szGuid;
		String name;
		String description;
	};

	static List<DeviceProperty> queryDeviceInfos()
	{
		List<DeviceProperty> list;
		HRESULT hr = DirectSoundEnumerateW(DeviceEnumProc, (VOID*)&list);
		if (FAILED(hr)) {
			logError("Can not query player device info");
		}
		return list;
	}

	static BOOL CALLBACK DeviceEnumProc(LPGUID lpGUID, LPCWSTR lpszDesc, LPCWSTR lpszDrvName, LPVOID lpContext)
	{
		List<DeviceProperty>& list = *((List<DeviceProperty>*)lpContext);
		DeviceProperty prop;
		if (lpGUID) {
			prop.guid = *lpGUID;
			prop.szGuid = Windows::getStringFromGUID(*lpGUID);
			prop.name = (sl_char16*)lpszDrvName;
			prop.description = (sl_char16*)lpszDesc;
			list.add(prop);
		}
		return TRUE;
	}
};

Ref<AudioPlayer> DirectSound::createPlayer(const AudioPlayerParam& param)
{
	return Ref<AudioPlayer>::from(_DirectSound_AudioPlayer::create(param));
}

List<AudioPlayerInfo> DirectSound::getPlayersList()
{
	List<AudioPlayerInfo> ret;
	ListLocker<_DirectSound_AudioPlayer::DeviceProperty> props(_DirectSound_AudioPlayer::queryDeviceInfos());
	for (sl_size i = 0; i < props.count(); i++) {
		_DirectSound_AudioPlayer::DeviceProperty& prop = props[i];
		AudioPlayerInfo info;
		info.id = prop.szGuid;
		info.name = prop.name;
		info.description = prop.description;
		ret.add(info);
	}
	return ret;
}
SLIB_MEDIA_NAMESPACE_END

#else

SLIB_MEDIA_NAMESPACE_BEGIN
Ref<AudioPlayer> DirectSound::createPlayer(const AudioPlayerParam& param)
{
	return Ref<AudioPlayer>::null();
}

List<AudioPlayerInfo> DirectSound::getPlayersList()
{
	return List<AudioPlayerInfo>::null();
}
SLIB_MEDIA_NAMESPACE_END

#endif