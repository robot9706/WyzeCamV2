INCLUDES     = -I../include/live555/UsageEnvironment/include/ -I../include/live555/groupsock/include/ -I../include/live555/liveMedia/include/ -I../include/live555/BasicUsageEnvironment/include -I../include/v4l2rtspserver/
LIBS = ../built-libs/libimp.so ../built-libs/libalog.so  ../built-libs/libimp.so ../built-libs/libliveMedia.so ../built-libs/libgroupsock.so ../built-libs/libUsageEnvironment.so ../built-libs/libBasicUsageEnvironment.so

CROSS_COMPILE=../toolchain/bin/mips-linux-gnu-

C            =         c
CPP          =           cpp
OBJ          =           o

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

CFLAGS = $(INCLUDES) -O2 -Wall -muclibc
CPLUSPLUSFLAGS = $(INCLUDES) -O2 -Wall -muclibc

LDFLAGS +=  -muclibc -lrt -lstdc++ -lpthread -ldl 

LINK_OBJ = isp_system.o isp_framesource.o isp_encoder.o isp_receive.o main.o
all: h264-tcp

.$(C).$(OBJ):
	$(CC) -c $(CFLAGS) $<

.$(CPP).$(OBJ):
	$(CPLUSPLUS) -c $(CPLUSPLUSFLAGS) $<

h264-tcp: $(LIBS) $(LINK_OBJ)
	$(CPLUSPLUS) $(LDFLAGS) -o $@ $^ $(LIBS) 

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(APP)

