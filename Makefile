CC = g++

ifeq ($(MemCheck), )
	MemCheck = None
endif

ifeq ($(MemPool), )
	MemPool = None
endif

ifeq ($(MemCycle), )
	MemCycle = None
endif

ifeq ($(Debug), )
	Debug = None
endif

CMS_MACRO_DEFINE =
#-fno-omit-frame-pointer 火焰图需要
#-fstack-protector 栈溢出会打印异常

ifeq ($(Debug), debug)
$(warning "making debug version")
CFLAGS = -ggdb \
	-o0
CMS_MACRO_DEFINE += -D__CMS_DEBUG__
else
CFLAGS = -ggdb \
	-o2
endif

#内存检测
ifeq ($(MemCheck),memcheck)
$(warning "making memory check version")
CMS_MACRO_DEFINE += -D_CMS_LEAK_CHECK_
endif
#使用内存池
ifeq ($(MemPool),mempool)
$(warning "making memory pool version")
CMS_MACRO_DEFINE += -D__CMS_POOL_MEM__
endif
#使用循环内存
ifeq ($(MemCycle),memcycle)
$(warning "making memory cycle version")
CMS_MACRO_DEFINE += -D__CMS_CYCLE_MEM__
endif

#public CFLAGS
CFLAGS += -Wall \
	-fstack-protector \
	-fno-omit-frame-pointer
	
#public Macro CFLAGS
CFLAGS += -D_REENTRANT \
	-D_POSIX_C_SOURCE=200112L \
	-D_FILE_OFFSET_BITS=64

#private Macro CFLAGS
#CFLAGS += -D_CMS_APP_USE_TIME_
CFLAGS += $(CMS_MACRO_DEFINE)
	

#include path CFLAGS
CFLAGS += -I./ \
	-I./third-party/linux/include

LINK = $(CC)

LDFLAGS = ./third-party/linux/libs/libssl.a \
	./third-party/linux/libs/libcrypto.a \
	./third-party/linux/libs/libev.a \
	./decode/libH264_5.a \
	-lpthread \
	-ldl \
	-lrt \
	-ls2n \
	-fPIC \
	-rdynamic

CSRCS = $(wildcard cJSON/*.c) 
CPPSRCS = $(wildcard app/*.cpp) \
		$(wildcard common/*.cpp) \
		$(wildcard config/*.cpp) \
		$(wildcard conn/*.cpp) \
		$(wildcard core/*.cpp) \
		$(wildcard dnscache/*.cpp) \
		$(wildcard enc/*.cpp) \
		$(wildcard flvPool/*.cpp) \
		$(wildcard interface/*.cpp) \
		$(wildcard log/*.cpp) \
		$(wildcard mem/*.cpp) \
		$(wildcard net/*.cpp) \
		$(wildcard protocol/*.cpp) \
		$(wildcard rfc/*.cpp) \
		$(wildcard static/*.cpp) \
		$(wildcard strategy/*.cpp) \
		$(wildcard taskmgr/*.cpp) \
		$(wildcard ts/*.cpp) \
		$(wildcard worker/*.cpp) \
		$(wildcard http/*.cpp)


COBJS := $(CSRCS:.c=.o)
CPPOBJS := $(CPPSRCS:.cpp=.o)

TARGET = ./bin/cms
	
all : $(TARGET)

$(COBJS) : %.o: %.c
	$(CC) $(CFLAGS) \
		-c $< -o $@ 
$(CPPOBJS) : %.o: %.cpp
	$(CC) $(CFLAGS) \
		-c $< -o $@ 
	
$(TARGET) : $(COBJS) $(CPPOBJS)
	@test -d './bin' || mkdir -p ./bin
	$(CC) -o $(TARGET) $(COBJS) $(CPPOBJS) $(LDFLAGS)

clean :
	rm -f $(COBJS) $(CPPOBJS) ./bin/cms

TARGETTYPE := APP
TARGETNAME := cms
