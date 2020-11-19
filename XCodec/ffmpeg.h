#pragma once
#include <cstdio>
#include <cstdlib>
#include <array>
#include "ClockTimer.h"
#include "FpsHelper.h"
#include "GlobalDefine.h"

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

inline char* av_err_to_str(int errnum)
{
	static char str[AV_ERROR_MAX_STRING_SIZE];
	memset(str, 0, sizeof(str));
	return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

static void ffmpeg_encode_frame_internal(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile, int32_t idx)
{
	/* send the frame to the encoder */
	/*if (frame)
		printf("[%d]Send frame %3I64d\n", idx, frame->pts);*/

	int ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		exit(1);
	}

	while (ret >= 0)
	{
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}

		//printf("[%d]Write packet %3I64d (size=%5d)\n", idx, pkt->pts, pkt->size);
		if (outfile != nullptr)
			fwrite(pkt->data, 1, pkt->size, outfile);
		av_packet_unref(pkt);
	}
}

inline void ffmpeg_encode_frame(int32_t idx, const char* filename, const char* codec_name, int32_t width, int32_t height, int64_t bit_rate)
{
	AVCodecContext *c = nullptr;
	int x, y;
	AVPacket *pkt;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };

	const AVCodec* codec = avcodec_find_encoder_by_name(codec_name);
	if (!codec) {
		fprintf(stderr, "Codec '%s' not found\n", codec_name);
		exit(1);
	}

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	pkt = av_packet_alloc();
	if (!pkt)
		exit(1);

	/* put sample parameters */
	c->bit_rate = bit_rate;
	/* resolution must be a multiple of two */
	c->width = width;
	c->height = height;
	/* frames per second */
	c->time_base = { 1, 50 };
	c->framerate = { 50, 1 };
	//c->thread_count = 32;

	/* emit one intra frame every ten frames
	 * check frame pict_type before passing frame
	 * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	 * then gop_size is ignored and the output of encoder
	 * will always be I frame irrespective to gop_size
	 */
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264)
	{
		av_opt_set(c->priv_data, "preset", "veryfast", 0);
		av_opt_set(c->priv_data, "profile", "main", 0);
	}

	/* open it */
	int ret = avcodec_open2(c, codec, nullptr);
	if (ret < 0) {
		fprintf(stderr, "Could not open codec: %s\n", av_err_to_str(ret));
		exit(1);
	}

	FILE* f = nullptr;
	if (filename != nullptr)
	{
		fopen_s(&f, filename, "wb");
		if (!f) {
			fprintf(stderr, "Could not open %s\n", filename);
			exit(1);
		}
	}

	/* prepare a dummy image */
	std::array<AVFrame*, 10> input_image{};
	for (size_t i = 0; i < input_image.size(); i++)
	{
		AVFrame*& frame = input_image.at(i);
		frame = av_frame_alloc();
		if (!frame) {
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		frame->format = c->pix_fmt;
		frame->width = c->width;
		frame->height = c->height;

		ret = av_frame_get_buffer(frame, 0);
		if (ret < 0) {
			fprintf(stderr, "Could not allocate the video frame data\n");
			exit(1);
		}

		/* make sure the frame data is writable */
		ret = av_frame_make_writable(frame);
		if (ret < 0)
			exit(1);

		/* prepare a dummy image */
		/* Y */
		for (y = 0; y < c->height; y++) {
			for (x = 0; x < c->width; x++) {
				frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
			}
		}

		/* Cb and Cr */
		for (y = 0; y < c->height / 2; y++) {
			for (x = 0; x < c->width / 2; x++) {
				frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
				frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
			}
		}

		fprintf(stderr, "[%d]prepare %I64u dummy image\n", idx, i);
	}

	const ClockTimer timer;
	ClockTimer frame_timer;
	FpsHelper fps_helper;
	int32_t fps = 0;
	fprintf(stderr, "encode start\n");
	const uint64_t nb_encode = 100;
	for (int i = 0; i < nb_encode; i++)
	{
		frame_timer.reset();
		AVFrame* frame = input_image.at(i % input_image.size());
		frame->pts = i;
		/* encode the image */
		ffmpeg_encode_frame_internal(c, frame, pkt, f, idx);
		fps = fps_helper.update_fps();
		fprintf(stderr, "[%d]encode one frame, cost %.2f ms, fps %d\n", idx, frame_timer.elapse_ms(), fps);
	}

	/* flush the encoder */
	ffmpeg_encode_frame_internal(c, nullptr, pkt, f, idx);
	fprintf(stderr, "=====[%d]encode stop, cost %.2f ms\n", idx, timer.elapse_ms());

	/* add sequence end code to have a real MPEG file */
	if ((codec->id == AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO) && f != nullptr)
		fwrite(endcode, 1, sizeof(endcode), f);
	if (f != nullptr)
		fclose(f);

	avcodec_free_context(&c);
	for (auto& frame : input_image)
		av_frame_free(&frame);
	av_packet_free(&pkt);
}