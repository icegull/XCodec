#pragma once
#include <cstdint>
#include <functional>

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

inline void generate_test_pattern(int32_t width, int32_t height, int32_t nb_frame, std::string_view dst_path)
{
	FILE* fout = nullptr;
	fopen_s(&fout, dst_path.data(), "wb");
	if (fout == nullptr)
		return;

	int x, y;
	for (size_t i = 0; i < nb_frame; i++)
	{
		/* prepare a dummy image */
		/* Y */
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				uint8_t value = x + y + i * 3;
				fwrite(&value, 1, 1, fout);
			}
		}

		/* Cb and Cr */
		for (y = 0; y < height / 2; y++) {
			for (x = 0; x < width / 2; x++) {
				uint8_t value = 128 + y + i * 2;
				fwrite(&value, 1, 1, fout);
				value = 64 + x + i * 5;
				fwrite(&value, 1, 1, fout);
			}
		}
		fprintf(stderr, "prepare %I64u dummy image\n", i);
	}
	fclose(fout);
}