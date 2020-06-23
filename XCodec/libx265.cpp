#include "libx265.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")

bool Libx265Encoder::initialize(const InitializeParams& _param, Encoder_CB _cb)
{
	return true;
}

bool Libx265Encoder::encode_frame_CPU(std::byte* pCpuFrame)
{
	return true;
}

bool Libx265Encoder::uninitialize()
{
	return true;
}