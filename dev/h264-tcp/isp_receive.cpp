#include "isp.h"
#include "log.h"

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
#include <sys/socket.h>
#include <netinet/ip.h>

bool isp_save_jpeg(isp_config *config, const string &pathPrefix)
{
	// Start encoder stream
	if (IMP_Encoder_StartRecvPic(config->encoderChannelIndex) < 0)
	{
		LOG("Failed to start encoder!");
		return false;
	}

	// Create timestamp
	time_t now;
	time(&now);
	struct tm *now_tm;
	now_tm = localtime(&now);
	char now_str[32];
	strftime(now_str, 40, "%Y%m%d%I%M%S", now_tm);

	// Build path
	char snap_path[256];
	sprintf(snap_path, "%s/snap-%s.jpg", pathPrefix.c_str(), now_str);

	// Open file
	LOG("Saving jpeg to: %s", snap_path);
	int snap_fd = open(snap_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (snap_fd < 0)
	{
		LOG("Failed to open file: %s", strerror(errno));
		return false;
	}

	// Start stream polling (1sec timeout)
	LOG("Polling stream");
	if (IMP_Encoder_PollingStream(config->encoderChannelIndex, 1000) < 0)
	{
		LOG("Polling stream timeout");
		close(snap_fd); // Close the file
		return false;
	}

	// Get stream
	IMPEncoderStream stream;
	LOG("Get stream");
	if (IMP_Encoder_GetStream(config->encoderChannelIndex, &stream, 1) < 0)
	{
		LOG("Failed to get Encoder stream!");
		close(snap_fd); // Close the file
		return false;
	}

	// Save encoder data to file
	LOG("Writing blocks");
	for (int i = 0; i < stream.packCount; i++)
	{
		// Write block
		if (write(snap_fd, (void *)stream.pack[i].virAddr, stream.pack[i].length) != stream.pack[i].length)
		{
			LOG("Failed to write stream block to file: %s\n", strerror(errno));
			break;
		}
	}

	// Release stream
	LOG("Release stream");
	IMP_Encoder_ReleaseStream(config->encoderChannelIndex, &stream);

	LOG("Done, closing");
	close(snap_fd); // Close output file

	// Stop encoder
	LOG("Stop Receive");
	if (IMP_Encoder_StopRecvPic(config->encoderChannelIndex) < 0)
	{
		LOG("Failed to stop Encoder!");
		return false;
	}

	return true;
}

bool isp_save_h264_limit(isp_config *config, const string &pathPrefix, uint64_t sizeLimit)
{
	// Start encoder stream
	if (IMP_Encoder_StartRecvPic(config->encoderChannelIndex) < 0)
	{
		LOG("Failed to start encoder!");
		return false;
	}

	// Create timestamp
	time_t now;
	time(&now);
	struct tm *now_tm;
	now_tm = localtime(&now);
	char now_str[32];
	strftime(now_str, 40, "%Y%m%d%I%M%S", now_tm);

	// Build path
	char snap_path[256];
	sprintf(snap_path, "%s/snap-%s.h264", pathPrefix.c_str(), now_str);

	// Open file
	LOG("Saving h264 to: %s", snap_path);
	int snap_fd = open(snap_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (snap_fd < 0)
	{
		LOG("Failed to open file: %s", strerror(errno));
		return false;
	}

	uint64_t written = 0;
	while (written < sizeLimit)
	{
		// Start stream polling (1sec timeout)
		LOG("Polling");
		if (IMP_Encoder_PollingStream(config->encoderChannelIndex, 1000) < 0)
		{
			LOG("Polling stream timeout");
			close(snap_fd); // Close the file
			return false;
		}

		// Get stream
		IMPEncoderStream stream;
		LOG("Get stream");
		if (IMP_Encoder_GetStream(config->encoderChannelIndex, &stream, 1) < 0)
		{
			LOG("Failed to get Encoder stream!");
			close(snap_fd); // Close the file
			return false;
		}

		// Save encoder data to file
		for (int i = 0; i < stream.packCount; i++)
		{
			// Write block
			int writeNum = write(snap_fd, (void *)stream.pack[i].virAddr, stream.pack[i].length);
			if (writeNum != stream.pack[i].length)
			{
				LOG("Failed to write stream block to file: %s\n", strerror(errno));
				break;
			}

			written += writeNum;
			LOG("Block size: %d", writeNum);
		}

		// Release stream
		LOG("Release stream");
		if (IMP_Encoder_ReleaseStream(config->encoderChannelIndex, &stream) < 0)
		{
			LOG("Release stream error");
			break;
		}
	}

	LOG("Close file");
	close(snap_fd); // Close output file

	// Stop encoder
	if (IMP_Encoder_StopRecvPic(config->encoderChannelIndex) < 0)
	{
		LOG("Failed to stop Encoder!");
		return false;
	}

	return true;
}

bool isp_h264_tcp(isp_config* config, int port)
{
	int socketHandle;
	struct sockaddr_in serveraddress;
	struct sockaddr_in clientaddress;

	// Create a TCP socket
	socketHandle = socket(AF_INET, SOCK_STREAM, 0);
	if (socketHandle < 0)
	{
		LOG("Failed to create TCP socket!");
		return false;
	}

	// Init address structures
	memset(&serveraddress, 0, sizeof(serveraddress));
	memset(&clientaddress, 0, sizeof(clientaddress));

	// Setup the TCP server
	serveraddress.sin_family = AF_INET;
	serveraddress.sin_addr.s_addr = INADDR_ANY;
	serveraddress.sin_port = htons(port);

	// Enable reuse address
	int enable = 1;
	if (setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		LOG("Failed to enable SO_REUSEADDR");
		return false;
	}

	// Bind socket
	if (bind(socketHandle, (const struct sockaddr *)&serveraddress, sizeof(sockaddr)) < 0)
	{
		LOG("Failed to bind socket!");
		return false;
	}

	// Start listening
	if (listen(socketHandle, 5) != 0)
	{
		LOG("Failed to start listening");
		return false;
	}

	LOG("TCP server listening on port %d", port);

	// Wait for a client
	LOG("Waiting for client...");

	socklen_t len = sizeof(clientaddress);
	int connfd = accept(socketHandle, (struct sockaddr *)&clientaddress, &len);
	if (connfd < 0)
	{
		LOG("Client accept error");
		return false;
	}

	// Start ISP stream
	LOG("Start encoder data");
	if (IMP_Encoder_StartRecvPic(config->encoderChannelIndex) < 0)
	{
		LOG("Failed to start encoder!");
		return false;
	}

	// Send packets until connection is open
	while (true)
	{
		int sockerror = 0;
		socklen_t errlen = sizeof(socklen_t);
		if (getsockopt(connfd, SOL_SOCKET, SO_ERROR, &sockerror, &errlen) < 0)
		{
			LOG("Failed to get TCP error???");
			break;
		}

		if (sockerror != 0)
		{
			LOG("Socket error %d", sockerror);
			break;
		}

		// Start stream polling (1sec timeout)
		LOG("Polling");
		if (IMP_Encoder_PollingStream(config->encoderChannelIndex, 1000) < 0)
		{
			LOG("Polling stream timeout");
			return false;
		}

		// Get stream
		IMPEncoderStream stream;
		LOG("Get stream");
		if (IMP_Encoder_GetStream(config->encoderChannelIndex, &stream, 1) < 0)
		{
			LOG("Failed to get Encoder stream!");
			return false;
		}

		// Save encoder data to file
		LOG("Write blocks");
		for (int i = 0; i < stream.packCount; i++)
		{
			// Write block
			int writeNum = write(connfd, (void *)stream.pack[i].virAddr, stream.pack[i].length);
			if (writeNum != stream.pack[i].length)
			{
				LOG("Failed to write stream block to file: %s\n", strerror(errno));
				break;
			}

			LOG("Block written, size: %d", writeNum);
		}

		// Release stream
		LOG("Release");
		if (IMP_Encoder_ReleaseStream(config->encoderChannelIndex, &stream) < 0)
		{
			LOG("Release stream error");
			break;
		}
	}

	// Stop ISP stream
	LOG("Stop encoder");
	if (IMP_Encoder_StopRecvPic(config->encoderChannelIndex) < 0)
	{
		LOG("Failed to stop Encoder!");
		return false;
	}

	// Close socket
	shutdown(connfd, SHUT_RDWR);
	close(connfd);

	// Close server
	close(socketHandle);
}