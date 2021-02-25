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

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>

#include <sys/ioctl.h>

// Sensor tools
#define IOCTL_SINFO_GET _IO(SENSOR_INFO_IOC_MAGIC, 100)

#define SENSOR_INFO_IOC_MAGIC 'S'
#define SENSOR_TYPE_INVALID -1
#define STRING_MAX_SIZE 256

int getSensorName()
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

// ISP init/exit
static IMPSensorInfo sensor_info;

static int init_isp()
{
	// ISP open
	int ret = IMP_ISP_Open();
	if(ret < 0){
		printf("failed to open ISP\n");
		return -1;
	}
	printf("ISP Open\n");

	// Init sensor info
	char sensorName[STRING_MAX_SIZE];
	int sensorId = getSensorName();
	int sensorAddr;

	if(sensorId == 1){
		strcpy(sensorName,"jxf22");
		sensorAddr = 0x40;
	} else if(sensorId == 2){
		strcpy(sensorName,"jxh62");
		sensorAddr = 0x30;
	}else{
		strcpy(sensorName,"jxf23");
		sensorAddr = 0x40;
	}
	printf("Found Sensor with ID: %d name=%s\n",  sensorId, sensorName);
	int sensorNameLen = strlen(sensorName);

	memset(&sensor_info, 0, sizeof(IMPSensorInfo)); // Init the struct
	memcpy(sensor_info.name, sensorName, sensorNameLen); // Copy sensor name
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, sensorName, sensorNameLen);
	sensor_info.i2c.addr = sensorAddr; // I2C address

	// Prepare sensor
	printf("ISP add sensor\n");
	ret = IMP_ISP_AddSensor(&sensor_info);
	if(ret < 0){
		printf("failed to AddSensor\n");
		return -1;
	}

	printf("ISP enable sensor\n");
	ret = IMP_ISP_EnableSensor();
	if(ret < 0){
		printf("failed to EnableSensor\n");
		return -1;
	}

	printf("ISP System init\n");
	ret = IMP_System_Init();
	if(ret < 0){
		printf("IMP_System_Init failed\n");
		return -1;
	}

	printf("ISp enable tuning\n");
	ret = IMP_ISP_EnableTuning();
	if(ret < 0){
		printf("IMP_ISP_EnableTuning failed\n");
		return -1;
	}

	return 0;
}

static int exit_isp()
{
	if (IMP_System_Exit() < 0) {
		printf("IMP System exit failed\n");
		return -1;
	}

	if (IMP_ISP_DisableSensor() < 0){
		printf("Failed to DisableSensor\n");
		return -1;
	}

	if (IMP_ISP_DelSensor(&sensor_info) < 0){
		printf("Failed to delete sensor\n");
		return -1;
	}

	if (IMP_ISP_DisableTuning() < 0){
		printf("IMP_ISP_DisableTuning failed\n");
		return -1;
	}

	if (IMP_ISP_Close()) {
		printf("Failed to close ISP\n");
		return -1;
	}

	return 0;
}

// Channel stuff
#define SENSOR_FRAME_RATE_NUM	25
#define SENSOR_FRAME_RATE_DEN	1

#define SENSOR_WIDTH	1920
#define SENSOR_HEIGHT	1080

#define SCALER_EN
#define SCALER_WIDTH	640
#define SCALER_HEIGHT	360

struct chn_conf{
	unsigned int index;
	unsigned int encoderIndex;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
	IMPFSChnAttr fs_chn_attr;
};

static chn_conf channel_config = 
{
	.index = 0,
	.encoderIndex = 2,
	.framesource_chn = { DEV_ID_FS, 0, 0},
	.imp_encoder = { DEV_ID_ENC, 0, 0},
	.fs_chn_attr = {
		.picWidth = SENSOR_WIDTH,
		.picHeight = SENSOR_HEIGHT,
		.pixFmt = PIX_FMT_NV12,
		.crop = { .enable = 0, .left = 0, .top = 0, .width = SENSOR_WIDTH, .height = SENSOR_HEIGHT },
		.scaler = { .enable = 0, .outwidth = SCALER_WIDTH, .outheight = SCALER_HEIGHT },
		.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
		.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
		.nrVBs = 3,
		.type = FS_PHY_CHANNEL
	}
};

static int create_jpeg_encoder(int groupIndex)
{
	IMPFSChnAttr* imp_chn_attr_tmp = &channel_config.fs_chn_attr;

	IMPEncoderCHNAttr channel_attr;

	memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));

	IMPEncoderAttr* enc_attr = &channel_attr.encAttr;
	enc_attr->enType = PT_JPEG;
	enc_attr->bufSize = 0;
	enc_attr->profile = 0;
	enc_attr->picWidth = imp_chn_attr_tmp->picWidth;
	enc_attr->picHeight = imp_chn_attr_tmp->picHeight;

	// Create Channel
	printf("Creating encoder channel");
	int ret = IMP_Encoder_CreateChn(channel_config.encoderIndex, &channel_attr);
	if (ret < 0) {
		printf("Failed to create encoder channel!\n");
		return -1;
	}

	// Resigter Channel
	printf("Registering encoder channel\n");
	ret = IMP_Encoder_RegisterChn(groupIndex, channel_config.encoderIndex);
	if (ret < 0) {
		printf("Failed to register encoder channel\n");
		return -1;
	}

	return 0;
}

static int destroy_jpeg_encoder(int group)
{
	if (IMP_Encoder_UnRegisterChn(channel_config.encoderIndex) < 0) {
		printf("Failed to unregister encoder channel\n");
		return -1;
	}

	if (IMP_Encoder_DestroyChn(channel_config.encoderIndex) < 0) {
		printf("Failed to destroy encoder channel!\n");
		return -1;
	}

	return 0;
}

static int init_framesource()
{
	printf("Creating channel\n");
	int ret = IMP_FrameSource_CreateChn(channel_config.index, &channel_config.fs_chn_attr);
	if (ret < 0) {
		printf("Failed to create channel!\n");
		return -1;
	}

	printf("Setting channel attrs\n");
	ret = IMP_FrameSource_SetChnAttr(channel_config.index, &channel_config.fs_chn_attr);
	if (ret < 0) {
		printf("Failed to set channel attrs!\n");
		return -1;
	}

	printf("Creating channel group\n");
	ret = IMP_Encoder_CreateGroup(channel_config.index);
	if (ret < 0) {
		printf("Failed to create channel group!\n");
		return -1;
	}

	// Create encoder
	if (create_jpeg_encoder(channel_config.index) != 0) {
		return -1;
	}

	// Bind
	printf("Binding\n");
	ret = IMP_System_Bind(&channel_config.framesource_chn, &channel_config.imp_encoder);
	if (ret < 0) {
		printf("Failed to bind\n");
		return -1;
	}

	// Enable
	printf("Enabling channel\n");
	ret = IMP_FrameSource_EnableChn(channel_config.index);
	if (ret < 0) {
		printf("Failed to enable channel!\n");
		return -1;
	}

	return 0;
}

static int destroy_framesource()
{
	if (IMP_FrameSource_DisableChn(channel_config.index) < 0) {
		printf("Failed to disable frame source\n");
		return -1;
	}

	if (IMP_System_UnBind(&channel_config.framesource_chn, &channel_config.imp_encoder) < 0) {
		printf("Failed to unbind\n");
		return -1;
	}

	if (destroy_jpeg_encoder(channel_config.index) < 0) {
		printf("Failed to destroy encoder\n");
		return -1;
	}

	if (IMP_Encoder_DestroyGroup(channel_config.index) < 0) {
		printf("Failed to destroy encoder group\n");
		return -1;
	}

	if (IMP_FrameSource_DestroyChn(channel_config.index) < 0) {
		printf("Failed to destroy framesource channel\n");
		return -1;
	}

	return 0;
}

// Take JPEG
static int save_stream(int fd, IMPEncoderStream *stream)
{
	int ret, i, nr_pack = stream->packCount;

	for (i = 0; i < nr_pack; i++) {
		ret = write(fd, (void *)stream->pack[i].virAddr,
					stream->pack[i].length);
		if (ret != stream->pack[i].length) {
			printf("stream write error:%s\n", strerror(errno));
			return -1;
		}
	}

	return 0;
}

int sample_do_get_jpeg_snap()
{
	int ret;

	// Start receive
	printf("JPEG receive\n");
	ret = IMP_Encoder_StartRecvPic(channel_config.encoderIndex);
	if (ret < 0) {
		printf("Failed to receive\n");
		return -1;
	}

	// Create timestamp
	time_t now;
	time(&now);
	struct tm *now_tm;
	now_tm = localtime(&now);
	char now_str[32];
	strftime(now_str, 40, "%Y%m%d%I%M%S", now_tm);

	// Create output path
	char snap_path[128];
	sprintf(snap_path, "/tmp/usb/snap-%s.jpg", now_str);

	// Open file
	printf("Creating output file: %s\n", snap_path);
	int snap_fd = open(snap_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (snap_fd < 0) {
		printf("failed: %s\n", strerror(errno));
		return -1;
	}

	// Start polling
	printf("Start polling\n");
	ret = IMP_Encoder_PollingStream(channel_config.encoderIndex, 1000);
	if (ret < 0) {
		printf("Polling failed!\n");
		return -1;
	}

	// Get snap
	printf("Getting snap\n");
	IMPEncoderStream stream;
	ret = IMP_Encoder_GetStream(channel_config.encoderIndex, &stream, 1);
	if (ret < 0) {
		printf("Snap failed\n");
		return -1;
	}
	printf("Snap OK\n");

	// Save stream
	ret = save_stream(snap_fd, &stream);
	if (ret < 0) {
		printf("Save failed\n");
		close(snap_fd);
		return ret;
	}

	// Cleanup
	printf("Cleanup\n");
	IMP_Encoder_ReleaseStream(channel_config.encoderIndex, &stream);

	close(snap_fd);

	// Stop
	printf("Stopping..\n");
	ret = IMP_Encoder_StopRecvPic(channel_config.encoderIndex);
	if (ret < 0) {
		printf("Stop failed\n");
		return -1;
	}

	return 0;
}

// Main
int main()
{
	printf("Helo\n");

	if (init_isp() != 0) {
		printf("ISP init fail\n");
		return -1;
	}
	printf("Init ISP DONE!\n");

	printf("Frame souce init\n");
	if (init_framesource() != 0) {
		printf("Fail\n");
		return -1;
	}

	sample_do_get_jpeg_snap();

	printf("Press anything to exit :>\n");
	getchar();

	destroy_framesource();

	exit_isp();

	return 0;
}
