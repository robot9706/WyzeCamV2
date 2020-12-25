#include "isp.h"
#include "log.h"

#include <memory.h>

static void create_encoder_attributes(IMPEncoderCHNAttr *attr, isp_config *config)
{
	IMPEncoderAttr *enc_attr = &attr->encAttr;
	IMPEncoderRcAttr *rc_attr = &attr->rcAttr;

	switch (config->mode)
	{
	case MODE_JPEG_SNAP:
	{
		LOG("Encoder JPEG");

		enc_attr->enType = PT_JPEG;
		enc_attr->bufSize = 0;
		enc_attr->profile = 0;
		enc_attr->picWidth = config->framesourceChannelAttributes.picWidth;
		enc_attr->picHeight = config->framesourceChannelAttributes.picHeight;
		break;
	}

	case MODE_H264_STREAM:
	{
		LOG("Encoder H264");

		enc_attr->enType = PT_H264;
		enc_attr->bufSize = 0;
		enc_attr->profile = 1;
		enc_attr->picWidth = config->framesourceChannelAttributes.picWidth;
		enc_attr->picHeight = config->framesourceChannelAttributes.picHeight;

		rc_attr->outFrmRate.frmRateNum = config->framesourceChannelAttributes.outFrmRateNum;
		rc_attr->outFrmRate.frmRateDen = config->framesourceChannelAttributes.outFrmRateDen;
		rc_attr->maxGop = 2 * rc_attr->outFrmRate.frmRateNum / rc_attr->outFrmRate.frmRateDen;

		rc_attr->attrRcMode.rcMode = ENC_RC_MODE_VBR;
		rc_attr->attrRcMode.attrH264Vbr.maxQp = 45;
		rc_attr->attrRcMode.attrH264Vbr.minQp = 15;
		rc_attr->attrRcMode.attrH264Vbr.staticTime = 2;
		rc_attr->attrRcMode.attrH264Vbr.maxBitRate = (double)2000.0 * (config->framesourceChannelAttributes.picWidth * config->framesourceChannelAttributes.picHeight) / (1280 * 720);
		rc_attr->attrRcMode.attrH264Vbr.iBiasLvl = 0;
		rc_attr->attrRcMode.attrH264Vbr.changePos = 80;
		rc_attr->attrRcMode.attrH264Vbr.qualityLvl = 2;
		rc_attr->attrRcMode.attrH264Vbr.frmQPStep = 3;
		rc_attr->attrRcMode.attrH264Vbr.gopQPStep = 15;
		rc_attr->attrRcMode.attrH264Vbr.gopRelation = false;

		rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
		rc_attr->attrHSkip.hSkipAttr.m = 0;
		rc_attr->attrHSkip.hSkipAttr.n = 0;
		rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
		rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
		rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
		rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		break;
	}

	default:
	{
		printf("Unknown mode!");
		break;
	}
	}
}

bool isp_encoder_init(isp_config *config)
{
	// Figure out channel attribute stuff
	IMPEncoderCHNAttr channel_attr;
	memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr)); // Init struct

	IMPEncoderAttr* enc_attr = &channel_attr.encAttr;
	enc_attr->enType = PT_JPEG;
	enc_attr->bufSize = 0;
	enc_attr->profile = 0;
	enc_attr->picWidth = config->framesourceChannelAttributes.picWidth;
	enc_attr->picHeight = config->framesourceChannelAttributes.picHeight;

	// Create encoder
	LOG("Creating encoder");

	if (IMP_Encoder_CreateGroup(config->encoderChannelGroupIndex) < 0)
	{
		LOG("Failed to create EncoderGroup!");
		return false;
	}

	if (IMP_Encoder_CreateChn(config->encoderChannelIndex, &channel_attr) < 0)
	{
		LOG("Failed to create Encoder!");
		return false;
	}

	// Assign Encoder to EncoderGroup
	if (IMP_Encoder_RegisterChn(config->encoderChannelGroupIndex, config->encoderChannelIndex) < 0)
	{
		LOG("Failed to assign Encoder to EncoderGroup");
		return false;
	}

	return true;
}

bool isp_encoder_destroy(isp_config *config)
{
	LOG("Destroying Encoder");

	if (IMP_Encoder_UnRegisterChn(config->encoderChannelIndex) < 0)
	{
		LOG("Failed to unassign Encoder from EncoderGroup");
		return false;
	}

	if (IMP_Encoder_DestroyChn(config->encoderChannelIndex) < 0)
	{
		LOG("Failed to destroy Encoder");
		return false;
	}

	if (IMP_Encoder_DestroyGroup(config->encoderChannelGroupIndex) < 0)
	{
		LOG("Failed to destroy EncoderGroup");
		return false;
	}

	return true;
}