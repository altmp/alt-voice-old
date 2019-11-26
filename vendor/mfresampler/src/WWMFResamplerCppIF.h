// 日本語。

#pragma once

#ifndef _MFRESAMPLER_STATIC
#ifdef WWMFRESAMPLER_EXPORTS
#define WWMFRESAMPLER_API extern "C"  __declspec(dllexport)
#else
#define WWMFRESAMPLER_API extern "C"  __declspec(dllimport)
#endif
#else
#define WWMFRESAMPLER_API
#endif

struct WWMFPcmFormatMarshal {
    int sampleFormat;       ///< WWMFBitFormatType of WWMFResampler.h
    int nChannels;          ///< PCMデータのチャンネル数。
    int bits;               ///< PCMデータ1サンプルあたりのビット数。パッド含む。
    int sampleRate;         ///< 44100等。
    int dwChannelMask;      ///< 2チャンネルステレオのとき3
    int validBitsPerSample; ///< PCMの量子化ビット数。
};

/// 新たに実体を作成。
/// @param halfFilterLength conversion quality. 1(min) to 60 (max)
/// @return 実体のID番号。instanceId 負の番号のときエラー。
WWMFRESAMPLER_API int __stdcall
WWMFResamplerInit(
    const WWMFPcmFormatMarshal *inputFmt,
    const WWMFPcmFormatMarshal *outputFmt,
    int halfFilterLength);

/// @param bytes buffer bytes. must be smaller than approx. 512KB to convert 44100Hz to 192000Hz
/// @param buffResult_inout リサンプル結果の置き場。呼び出し側が十分大きい領域を確保して渡す。
/// @param resultBytes_inout buff_returnのバイト数を渡して呼び出す。実際に書き込まれたバイト数が戻る。
/// @param instanceId 実体のID番号。Initで戻る値。
WWMFRESAMPLER_API int __stdcall
WWMFResamplerResample(int instanceId, const unsigned char *buff, int bytes, unsigned char * buffResult_inout, int * resultBytes_inout);

/// 最後の入力データをResample()に送ったあとに1回呼ぶ。
/// 変換パイプラインに溜まった残り滓(必要入力サンプルが足りないので計算がペンディングしていたデータ)が出てくる。
WWMFRESAMPLER_API int __stdcall
WWMFResamplerDrain(int instanceId, unsigned char * buffResult_inout, int * resultBytes_inout);

/// 実体を削除する。
/// @param instanceId 実体のID番号。Initで戻る値。
WWMFRESAMPLER_API int __stdcall
WWMFResamplerTerm(int instanceId);
