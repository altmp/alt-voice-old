// 日本語 UTF-8

#include "WWUtil.h"

void
WWWaveFormatDebug(WAVEFORMATEX *v)
{
    (void)v;

    dprintf(
        "  cbSize=%d\n"
        "  nAvgBytesPerSec=%d\n"
        "  nBlockAlign=%d\n"
        "  nChannels=%d\n"
        "  nSamplesPerSec=%d\n"
        "  wBitsPerSample=%d\n"
        "  wFormatTag=0x%x\n",
        v->cbSize,
        v->nAvgBytesPerSec,
        v->nBlockAlign,
        v->nChannels,
        v->nSamplesPerSec,
        v->wBitsPerSample,
        v->wFormatTag);
}

void
WWWFEXDebug(WAVEFORMATEXTENSIBLE *v)
{
    (void)v;

    dprintf(
        "  dwChannelMask=0x%x\n"
        "  Samples.wValidBitsPerSample=%d\n"
        "  SubFormat=%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x\n",
        v->dwChannelMask,
        v->Samples.wValidBitsPerSample,
        v->SubFormat.Data1,
        v->SubFormat.Data2,
        v->SubFormat.Data3,
        v->SubFormat.Data4[0],
        v->SubFormat.Data4[1],
        v->SubFormat.Data4[2],
        v->SubFormat.Data4[3],
        v->SubFormat.Data4[4],
        v->SubFormat.Data4[5],
        v->SubFormat.Data4[6],
        v->SubFormat.Data4[7]
        );
}

