#pragma once
#include <cstdint>
#include <functional>

extern "C"
{
#include <libavutil/error.h>
}

enum class PFrameInterval
{
	I = 0,
	IPP = 1,
	IBP = 2,
	IBBP = 3
};

struct InitializeParams
{
	int16_t         camera_id = -1;
	int32_t			gpu_idx = 0;
	uint32_t		width = 1920;
	uint32_t		height = 1080;
	uint32_t		frame_rate_num = 1;
	uint32_t		frame_rate_den = 25;
	uint32_t		gop_length = 1;
	PFrameInterval	frame_interval_p = PFrameInterval::I;
	uint32_t		bit_rate = 0;
	bool			bH264 = true;
	bool			bCabacMode = true;
};

enum class FrameType
{
	P = 0x0,     /**< Forward predicted */
	B = 0x01,    /**< Bi-directionally predicted picture */
	I = 0x02,    /**< Intra predicted picture */
	IDR = 0x03,    /**< IDR picture */
	BI = 0x04,    /**< Bi-directionally predicted with only Intra MBs */
	SKIPPED = 0x05,    /**< Picture is skipped */
	INTRA_REFRESH = 0x06,    /**< First picture in intra refresh cycle */
	UNKNOWN = 0xFF     /**< Picture type unknown */
};

using Encoder_CB = std::function<void(void* bitstreamBufferPtr, int32_t bitstreamSizeInBytes, int32_t frameIdx, uint64_t outputTimeStamp, int32_t nType)>;

inline char* av_err_to_str(int errnum)
{
	static char str[AV_ERROR_MAX_STRING_SIZE];
	memset(str, 0, sizeof(str));
	return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}