#ifndef __ISP__
#define __ISP__

#include "log.h"

#include <string>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>

using namespace std;

#define NUM_FRAMESOURCE_CHANNELS 2

#define MODE_JPEG_SNAP 0
#define MODE_H264_STREAM 1

struct isp_config
{
	unsigned int framesourceChannelIndex;
	unsigned int encoderChannelIndex;
    unsigned int encoderChannelGroupIndex;
	IMPFSChnAttr framesourceChannelAttributes;
	IMPCell framesourceCell;
	IMPCell encoderCell;
	int mode;
};

bool isp_init();
bool isp_destroy();

bool isp_framesource_init(isp_config* config);
bool isp_framesource_destroy(isp_config* config);

bool isp_framesource_on(isp_config* config);
bool isp_framesource_off(isp_config* config);

bool isp_encoder_init(isp_config* config);
bool isp_encoder_destroy(isp_config* config);

bool isp_bind_fs2encoder(isp_config* config);
bool isp_unbind_fs2encoder(isp_config* config);

bool isp_save_jpeg(isp_config* config, const string& pathPrefix);
bool isp_save_h264_limit(isp_config* config, const string& pathPrefix, uint64_t sizeLimit);

bool isp_h264_tcp(isp_config* config, int port);

#endif