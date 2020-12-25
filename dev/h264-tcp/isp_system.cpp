#include "isp.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/ioctl.h>

#define IOCTL_SINFO_GET _IO(SENSOR_INFO_IOC_MAGIC, 100)
#define SENSOR_INFO_IOC_MAGIC 'S'
#define SENSOR_TYPE_INVALID -1
#define STRING_MAX_SIZE 256

static IMPSensorInfo sensor_info;

static int getSensorName()
{
	int ret  = 0;
	int fd   = 0;
	int data = -1;

	fd = open("/dev/sinfo", O_RDWR);
	if (-1 == fd) {
		printf("Failed to open sensor info!\n");
		return -1;
	}

	// Using iotcl to get sensor info.
	// cmd is IOCTL_SINFO_GET, data note sensor type according to SENSOR_TYPE
	ret = ioctl(fd, IOCTL_SINFO_GET, &data);
	close(fd);

	if (0 != ret) {

		printf("SensorInfo IOCTL failed\n");
		return -1;
	}
	if (SENSOR_TYPE_INVALID == data)
	{
		printf("Got invalid sensor data!\n");
		return -1;
	}

	return data;
}

bool isp_init()
{
    // ISP open
	if (IMP_ISP_Open() < 0){
		LOG("Failed to open ISP");
		return false;
	}
	LOG("ISP Open");

	// Init sensor info
	char sensorName[STRING_MAX_SIZE];
	int sensorId = getSensorName();
	int sensorAddr;

	if (sensorId == 1)
    {
		strcpy(sensorName,"jxf22");
		sensorAddr = 0x40;
	}
    else if (sensorId == 2)
    {
		strcpy(sensorName,"jxh62");
		sensorAddr = 0x30;
	}
    else
    {
		strcpy(sensorName,"jxf23");
		sensorAddr = 0x40;
	}
	LOG("Found Sensor with ID: %d name=%s\n",  sensorId, sensorName);
	int sensorNameLen = strlen(sensorName);

    // Init the sensor info struct
	memset(&sensor_info, 0, sizeof(IMPSensorInfo)); // Init the struct
	memcpy(sensor_info.name, sensorName, sensorNameLen); // Copy sensor name
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, sensorName, sensorNameLen);
	sensor_info.i2c.addr = sensorAddr; // I2C address

	// Prepare sensor
	if (IMP_ISP_AddSensor(&sensor_info) < 0)
    {
		LOG("failed to AddSensor");
		return false;
	}

	if (IMP_ISP_EnableSensor() < 0)
    {
		LOG("failed to EnableSensor");
		return false;
	}

	if (IMP_System_Init() < 0)
    {
		LOG("IMP_System_Init failed");
		return false;
	}

	if (IMP_ISP_EnableTuning() < 0)
    {
		LOG("IMP_ISP_EnableTuning failed");
		return false;
	}

    return true;
}

bool isp_destroy()
{
    LOG("Destroying ISP system");
	if (IMP_ISP_DisableTuning() < 0)
    {
		LOG("Failed to disable tuning");
		return false;
	}

	if (IMP_System_Exit() < 0)
    {
        LOG("Failed to destroy system");
        return false;
    }

	if (IMP_ISP_DisableSensor() < 0)
    {
		LOG("Failed to disable sensor");
		return false;
	}

	if (IMP_ISP_DelSensor(&sensor_info) < 0)
    {
		LOG("Failed to delete sensor");
		return false;
	}

	if (IMP_ISP_Close())
    {
		LOG("Failed to close ISP");
		return false;
	}

	LOG("ISP system destroyed");

	return true;
}

bool isp_bind_fs2encoder(isp_config* config)
{
	return (IMP_System_Bind(&config->framesourceCell, &config->encoderCell) == 0);
}

bool isp_unbind_fs2encoder(isp_config* config)
{
	return (IMP_System_UnBind(&config->framesourceCell, &config->encoderCell) == 0);
}