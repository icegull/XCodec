#pragma once
#include "GlobalDefine.h"

struct AVPacket;
struct AVFrame;
struct AVCodec;
struct AVCodecContext;
class Libx265Encoder
{
	AVFrame*			m_yuv420p_av_frame = nullptr;
	InitializeParams	m_param;
	AVCodec*			m_encoder = nullptr;
	AVCodecContext*		m_encoder_cxt = nullptr;
	AVPacket*			m_encoded_pkt = nullptr;
	Encoder_CB			m_cb;

public:
	Libx265Encoder() = default;
	virtual ~Libx265Encoder() = default;

	bool initialize(const InitializeParams& _param, Encoder_CB&& _cb);
	bool encode_frame_CPU(uint8_t* pCpuFrame);
	bool uninitialize();
};