#include <stdio.h>
#include <unistd.h>

#include "isp.h"
#include "log.h"

#define SENSOR_WIDTH 640
#define SENSOR_HEIGHT 480

#define SENSOR_FRAME_RATE_NUM 25
#define SENSOR_FRAME_RATE_DEN 1

#define SCALER_WIDTH 640
#define SCALER_HEIGHT 480

static isp_config isp_config =
{
    .framesourceChannelIndex = 0, // First FS channel
    .encoderChannelIndex = NUM_FRAMESOURCE_CHANNELS + 0, // First encoder channel
    .encoderChannelGroupIndex = 0, // First group index
    .framesourceChannelAttributes =
    {
        .picWidth = SENSOR_WIDTH,
		.picHeight = SENSOR_HEIGHT,
		.pixFmt = PIX_FMT_NV12,
		.crop = { .enable = 1, .left = 0, .top = 0, .width = SENSOR_WIDTH, .height = SENSOR_HEIGHT },
		.scaler = { .enable = 0, .outwidth = SCALER_WIDTH, .outheight = SCALER_HEIGHT },
		.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
		.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
		.nrVBs = 3,
		.type = FS_PHY_CHANNEL
    },
    .framesourceCell = { DEV_ID_FS, 0, 0 },
    .encoderCell = { DEV_ID_ENC, 0, 0 },
    .mode = -1
};

static void streaming()
{
    // Enable the Framesource
    isp_framesource_on(&isp_config);

    switch (isp_config.mode)
    {
        case MODE_JPEG_SNAP:
        {
            // Save JPEG
            isp_save_jpeg(&isp_config, "/tmp/usb");
            break;
        }

        case MODE_H264_STREAM:
        {
            //isp_save_h264_limit(&isp_config, "/tmp/usb", 1024 * 1024 * 5);

            isp_h264_tcp(&isp_config, 9000);
            break;
        }
    }

    // Disable framesource
    isp_framesource_off(&isp_config);
}

static void printhelp()
{
    LOG("-j: Take a JPEG snap");
    LOG("-s: Start a H264 TCP stream");
}

static bool process_args(int argc, char **argv)
{
    int c;
    while ((c = getopt(argc, argv, "jsh")) != -1)
    {
        switch (c)
        {
        case 'h':
        {
            printhelp();
            return false;
        }

        case 'j':
        {
            if (isp_config.mode == -1)
            {
                isp_config.mode = MODE_JPEG_SNAP;
            }
            else
            {
                printf("Multiple modes defined!");
                return false;
            }
            
            break;
        }

        case 's':
        {
            if (isp_config.mode == -1)
            {
                isp_config.mode = MODE_H264_STREAM;
            }
            else
            {
                printf("Multiple modes defined!");
                return false;
            }
            
            break;
        }
        }
    }

    return true;
}

int main(int argc, char **argv)
{
    if (!process_args(argc, argv))
    {
        return -1;
    }

    if (isp_config.mode == -1)
    {
        isp_config.mode = MODE_H264_STREAM; // Default to H264 streaming
    }

    // 1) Init ISP
    if (!isp_init())
    {
        return -1;
    }

    // 2) Create frame source
    if (!isp_framesource_init(&isp_config))
    {
        return -1;
    }

    // 3) Create encoder and encoder group
    if (!isp_encoder_init(&isp_config))
    {
        return -1;
    }

    // 4) Direct framesource to encoder cell
    if (!isp_bind_fs2encoder(&isp_config))
    {
        return -1;
    }

    // Stream :>
    streaming();

    // Cleanup
    {
        // Disconnect framesource from encoder
        isp_unbind_fs2encoder(&isp_config);

        // Destroy encoder and encoder group
        isp_encoder_destroy(&isp_config);

        // Destroy frame source
        isp_framesource_destroy(&isp_config);

        // Destroy isp
        isp_destroy();
    }

    return 0;
}