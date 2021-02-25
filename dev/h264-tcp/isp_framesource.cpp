#include "isp.h"
#include "log.h"

bool isp_framesource_init(isp_config* config)
{
    LOG("Creating FrameSource");

    if (IMP_FrameSource_CreateChn(config->framesourceChannelIndex, &config->framesourceChannelAttributes) < 0)
    {
        LOG("Failed to create FrameSource channel");
        return false;
    }

    if (IMP_FrameSource_SetChnAttr(config->framesourceChannelIndex, &config->framesourceChannelAttributes) < 0)
    {
        LOG("Failed to set FrameSource attributes");
        return false;
    }

    return true;
}

bool isp_framesource_destroy(isp_config* config)
{
    if (IMP_FrameSource_DestroyChn(config->framesourceChannelIndex) < 0)
    {
        LOG("Failed to destroy FrameSource channel");
        return false;
    }

    return true;
}

bool isp_framesource_on(isp_config* config)
{
    return (IMP_FrameSource_EnableChn(config->framesourceChannelIndex) == 0);
}

bool isp_framesource_off(isp_config* config)
{
    return (IMP_FrameSource_DisableChn(config->framesourceChannelIndex) == 0);
}