// 日本語。

#define WWMFRESAMPLER_EXPORTS
#include "WWMFResamplerCppIF.h"
#include "WWMFResampler.h"

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <map>

static int gNextInstanceId = 300;
static std::map<int, WWMFResampler*> gInstances;

static WWMFResampler *
FindInstance(int idx)
{
    auto ite = gInstances.find(idx);
    if (ite == gInstances.end()) {
        return nullptr;
    }

    return ite->second;
}

#define FIND_INSTANCE                   \
    HRESULT hr = S_OK;                  \
    auto *p = FindInstance(instanceId); \
    if (nullptr == p) {                 \
        return E_INVALIDARG;            \
    }

WWMFRESAMPLER_API int __stdcall
WWMFResamplerInit(
        const WWMFPcmFormatMarshal *inPFM,
        const WWMFPcmFormatMarshal *outPFM,
        int halfFilterLength)
{
    HRESULT hr = S_OK;
    WWMFPcmFormat inPF((WWMFBitFormatType)inPFM->sampleFormat, inPFM->nChannels, inPFM->bits, inPFM->sampleRate, inPFM->dwChannelMask, inPFM->validBitsPerSample);
    WWMFPcmFormat outPF((WWMFBitFormatType)outPFM->sampleFormat, outPFM->nChannels, outPFM->bits, outPFM->sampleRate, outPFM->dwChannelMask, outPFM->validBitsPerSample);

    auto *p = new WWMFResampler;
    if (nullptr == p) {
        return E_OUTOFMEMORY;
    }

    hr = p->Initialize(inPF, outPF, halfFilterLength);
    if (FAILED(hr)) {
        return hr;
    }

    // 成功。
    int instanceId = gNextInstanceId++;
    gInstances.insert(std::pair<int, WWMFResampler*>(instanceId, p));

    return instanceId;
}

WWMFRESAMPLER_API int __stdcall
WWMFResamplerTerm(int instanceId)
{
    FIND_INSTANCE;

    p->Finalize();

    delete p;
    p = nullptr;
    gInstances.erase(instanceId);

    return S_OK;
}

WWMFRESAMPLER_API int __stdcall
WWMFResamplerResample(int instanceId, const unsigned char *buff, int bytes, unsigned char * buffResult_inout, int * resultBytes_inout)
{
    FIND_INSTANCE;

    WWMFSampleData sd;
    hr = p->Resample(buff, bytes, &sd);
    if (FAILED(hr)) {
        return hr;
    }

    if (*resultBytes_inout < (int)sd.bytes) {
        sd.Release();
        return E_INVALIDARG;
    }

    memcpy(buffResult_inout, sd.data, sd.bytes);
    *resultBytes_inout = sd.bytes;

    sd.Release();

    return hr;
}

/// 最後の入力データをResample()に送ったあとに1回呼ぶ。
/// バッファに溜まった残り滓(必要入力サンプルが足りないので計算がペンディングしていたデータ)が出てくる。
WWMFRESAMPLER_API int __stdcall
WWMFResamplerDrain(int instanceId, unsigned char * buffResult_inout, int * resultBytes_inout)
{
    FIND_INSTANCE;

    WWMFSampleData sd;
    hr = p->Drain(*resultBytes_inout, &sd);
    if (FAILED(hr)) {
        return hr;
    }

    if (sd.bytes == 0) {
        sd.Release();
        *resultBytes_inout = 0;
        return S_OK;
    }

    if (*resultBytes_inout < (int)sd.bytes) {
        sd.Release();
        return E_INVALIDARG;
    }

    memcpy(buffResult_inout, sd.data, sd.bytes);
    *resultBytes_inout = sd.bytes;

    sd.Release();

    return hr;
}
