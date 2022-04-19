//-----------------------------------------------------------
// Record an audio stream from the default audio capture
// device. The RecordAudioStream function allocates a shared
// buffer big enough to hold one second of PCM audio data.
// The function uses this buffer to stream data from the
// capture device. The main loop runs every 1/2 second.
//-----------------------------------------------------------

#include "recordAudio.h"
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <functiondiscoverykeys_devpkey.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <atlstr.h>

using namespace std;
std::mutex mtx;
void captureAudio::writeAudio(BYTE *data_, int size_)
{
	mtx.lock();
	size = size_;
	//if (data)delete[]data;
	//data = new BYTE[size];
	memcpy(data, data_, size);
	mtx.unlock();
}
void captureAudio::readAudio(BYTE* data_, int& size_)
{
	mtx.lock();
	size_ = size;
	memcpy(data_, data, size);
	mtx.unlock();
}


captureAudio actualAudioData;
int running = TRUE;


// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
class MyAudioSink
{
public:
	HRESULT SetFormat(WAVEFORMATEX *pWF);
	HRESULT CopyData(BYTE* pData, UINT32 NumFrames, BOOL *pDone);
};


HRESULT MyAudioSink::SetFormat(WAVEFORMATEX * pWF)
{
	// For the time being, just return OK.
	return (S_OK);
}

HRESULT MyAudioSink::CopyData(BYTE * pData, UINT32 NumFrames, BOOL * pDone)
{
	static int CallCount = 0;
	//cout << "CallCount = " << CallCount++ << endl ;
	// For the time being, just pretend to record for 15 seconds.
	if (clock() > 15 * CLOCKS_PER_SEC) *pDone = true;
	else *pDone = false;
	return S_OK;
}
unsigned char *g_data = NULL;
HRESULT RecordAudioStream(MyAudioSink *pMySink);
void start_recording()
{
	MyAudioSink pMySink;
	RecordAudioStream(&pMySink);
}

string GetDeviceName(IMMDeviceCollection *DeviceCollection, UINT DeviceIndex, LPWSTR &pdeviceId)
{
	IMMDevice *device;
	LPWSTR deviceId;
	HRESULT hr;

	hr = DeviceCollection->Item(DeviceIndex, &device);
	//EXIT_ON_ERROR(hr)

	hr = device->GetId(&deviceId);
	//EXIT_ON_ERROR(hr)

	IPropertyStore *propertyStore;
	hr = device->OpenPropertyStore(STGM_READ, &propertyStore);
	//SAFE_RELEASE(&device);
//	EXIT_ON_ERROR(hr)

	PROPVARIANT friendlyName;
	PropVariantInit(&friendlyName);
	hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
	//SAFE_RELEASE(&propertyStore);
	//EXIT_ON_ERROR(hr)

	string Result,a,b;
	Result = CW2A(friendlyName.pwszVal);
	if (Result.find("Stereo Mix")==0)
		pdeviceId = deviceId;
	a = "#######";
	b =CW2A(deviceId);
	Result += a + b;
	PropVariantClear(&friendlyName);
	CoTaskMemFree(deviceId);

	return Result;
}



HRESULT RecordAudioStream(MyAudioSink *pMySink)
{
	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	UINT32 bufferFrameCount;
	UINT32 numFramesAvailable;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	IAudioClient *pAudioClient = NULL;
	IAudioCaptureClient *pCaptureClient = NULL;
	IAudioRenderClient *pRenderClient = NULL;
	WAVEFORMATEX *pwfx = NULL;
	WAVEFORMATEXTENSIBLE *wfe = NULL;
	UINT32 packetLength = 0;
	BOOL bDone = FALSE;
	BYTE *pData;
	DWORD flags;
	hr = CoInitialize(NULL);
	IMMDeviceCollection  *pEndpoints = NULL;
	EXIT_ON_ERROR(hr)

		hr = CoCreateInstance(
			CLSID_MMDeviceEnumerator, NULL,
			CLSCTX_ALL, IID_IMMDeviceEnumerator,
			(void**)&pEnumerator);
	EXIT_ON_ERROR(hr)

		hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pEndpoints);
	UINT deviceCount;
	hr = pEndpoints->GetCount(&deviceCount);
	EXIT_ON_ERROR(hr)
		LPWSTR deviceId=0;
	for (UINT DeviceIndex = 0; DeviceIndex < deviceCount; DeviceIndex++)
	{
		string deviceName = GetDeviceName(pEndpoints, DeviceIndex, deviceId);
		cout << DeviceIndex <<": " << deviceName.c_str()<< endl;
	}
	EXIT_ON_ERROR(hr)
		hr = pEnumerator->GetDefaultAudioEndpoint(
			eCapture, eConsole, &pDevice); 
	hr = pEnumerator->GetDevice(deviceId,&pDevice);
	EXIT_ON_ERROR(hr)

		hr = pDevice->Activate(
			IID_IAudioClient, CLSCTX_ALL,
			NULL, (void**)&pAudioClient);
	EXIT_ON_ERROR(hr)
		
		hr = pAudioClient->GetMixFormat(&pwfx);
	wfe = (WAVEFORMATEXTENSIBLE*)(pwfx);


	EXIT_ON_ERROR(hr)

		hr = pAudioClient->Initialize(
			AUDCLNT_SHAREMODE_SHARED,
			0,
			hnsRequestedDuration,
			0,
			pwfx,
			NULL);
	EXIT_ON_ERROR(hr)

		// Get the size of the allocated buffer.
		hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr)

		hr = pAudioClient->GetService(
			IID_IAudioCaptureClient,
			(void**)&pCaptureClient);
	EXIT_ON_ERROR(hr)

		// Notify the audio sink which format to use.
		hr = pMySink->SetFormat(pwfx);
	EXIT_ON_ERROR(hr)

		// Calculate the actual duration of the allocated buffer.
		hnsActualDuration = (double)REFTIMES_PER_SEC *
		bufferFrameCount / pwfx->nSamplesPerSec;

	hr = pAudioClient->Start();  // Start recording.
	EXIT_ON_ERROR(hr)

		
		// Each loop fills about half of the shared buffer.
		while (running == TRUE)
		{
			BYTE data[MAXS];
			int actualsize = 0;
			// Sleep for half the buffer duration.
			int duration = hnsActualDuration / REFTIMES_PER_MILLISEC / 20;
			Sleep(duration*4);

			hr = pCaptureClient->GetNextPacketSize(&packetLength);
			EXIT_ON_ERROR(hr)
			numFramesAvailable = 0;
				while (packetLength != 0)
				{
					// Get the available data in the shared buffer.
					hr = pCaptureClient->GetBuffer(
						&pData,
						&numFramesAvailable,
						&flags, NULL, NULL);
					EXIT_ON_ERROR(hr)

						if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
						{
							pData = NULL;  // Tell CopyData to write silence.
						}

					// Copy the available capture data to the audio sink.
					//hr = pMySink->CopyData(
						//pData, numFramesAvailable, &bDone);

					memcpy(data + actualsize, pData, numFramesAvailable);
					actualsize += numFramesAvailable;

					EXIT_ON_ERROR(hr)

						hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
					EXIT_ON_ERROR(hr)

						hr = pCaptureClient->GetNextPacketSize(&packetLength);
					EXIT_ON_ERROR(hr)


							/*for (int ii = 0; ii < numFramesAvailable; ii+=8)
							{

								float *f2 = (float*)&pData[ii + 4];
								float *f1 = (float*)&pData[ii + 0];
								cout << *f1 << ", " << *f2 << endl;

								break;
							}*/
						if ((actualsize - numFramesAvailable) >= MAXSAMPLE)
						{
							break;
						}
				}
			actualAudioData.writeAudio(data, actualsize - numFramesAvailable);
		}

	hr = pAudioClient->Stop();  // Stop recording.
	EXIT_ON_ERROR(hr)

		Exit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator)
		SAFE_RELEASE(pDevice)
		SAFE_RELEASE(pAudioClient)
		SAFE_RELEASE(pCaptureClient)

		return hr;
}
