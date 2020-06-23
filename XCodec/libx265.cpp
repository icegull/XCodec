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

bool Libx265Encoder::initialize(const InitializeParams& _param, Encoder_CB&& _cb)
{
	av_log_set_level(AV_LOG_QUIET);
	m_param = _param;
	m_cb = _cb;

	m_yuv420p_av_frame = av_frame_alloc();
	m_encoder = avcodec_find_encoder_by_name("libx264");
	if (m_encoder == nullptr)
		return false;

	m_encoder_cxt = avcodec_alloc_context3(m_encoder);
	if (m_encoder_cxt == nullptr)
		return false;

	m_encoder_cxt->bit_rate = _param.bit_rate;
	m_encoder_cxt->width = _param.width;
	m_encoder_cxt->height = _param.height;
	m_encoder_cxt->time_base.num = _param.frame_rate_den;
	m_encoder_cxt->time_base.den = _param.frame_rate_num;
	m_encoder_cxt->framerate.num = _param.frame_rate_num;
	m_encoder_cxt->framerate.den = _param.frame_rate_den;
	m_encoder_cxt->pix_fmt = AV_PIX_FMT_YUV420P;
	m_encoder_cxt->gop_size = _param.gop_length;
	m_encoder_cxt->has_b_frames = 0;
	m_encoder_cxt->profile = FF_PROFILE_H264_MAIN;
	m_encoder_cxt->codec_type = AVMEDIA_TYPE_VIDEO;
	m_encoder_cxt->codec_id = AV_CODEC_ID_H264;

	//preset: ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow, placebo
	av_opt_set(m_encoder_cxt->priv_data, "preset", "veryfast", 0);
	//tune: film, animation, grain, stillimage, psnr, ssim, fastdecode, zerolatency
	av_opt_set(m_encoder_cxt->priv_data, "tune", "zerolatency", 0);
	//profile: baseline, main, high, high10, high422, high444
	av_opt_set(m_encoder_cxt->priv_data, "profile", "main", 0);

	av_opt_set(m_encoder_cxt->priv_data, "x264-params", "sliced-threads=0", 0);

	av_opt_set(m_encoder_cxt->priv_data, "x264opts", "interlaced=1", 0);

	int ret = avcodec_open2(m_encoder_cxt, m_encoder, nullptr);
	if (ret < 0)
	{
		char errormsg[255];
		av_strerror(ret, errormsg, 255);
		return false;
	}
	m_encoded_pkt = av_packet_alloc();
	return m_encoded_pkt != nullptr;
}

bool Libx265Encoder::encode_frame_CPU(uint8_t* pCpuFrame)
{
	int ret = av_image_fill_arrays(m_yuv420p_av_frame->data, m_yuv420p_av_frame->linesize, pCpuFrame, AV_PIX_FMT_YUV420P, m_param.width, m_param.height, 1);
	if (ret < 0)
		return false;

	ret = avcodec_send_frame(m_encoder_cxt, m_yuv420p_av_frame);
	if (ret < 0)
		return false;

	while (ret >= 0)
	{
		ret = avcodec_receive_packet(m_encoder_cxt, m_encoded_pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return true;
		if (ret < 0)
			return false;
		m_cb(m_encoded_pkt->data, m_encoded_pkt->size, 0, m_encoded_pkt->pts,
			m_encoded_pkt->flags == AV_PKT_FLAG_KEY ? 0x03 : 0);

		av_packet_unref(m_encoded_pkt);
	}

	return true;
}

bool Libx265Encoder::uninitialize()
{
	av_frame_free(&m_yuv420p_av_frame);
	avcodec_free_context(&m_encoder_cxt);
	av_packet_free(&m_encoded_pkt);
	return true;
}