CC = g++

ifeq ($(MemCheck), )
	MemCheck = None
endif

ifeq ($(Debug), )
	Debug = None
endif

ifeq ($(CONFIG), debug)
$(warning "making debug version")
CFLAGS = -ggdb \
	-o0 \
	-rdynamic \
	-D__CMS_DEBUG__
else
CFLAGS = -ggdb \
	-o2 \
	-rdynamic
endif

ifeq ($(MemCheck),memcheck)
$(warning "making memory check version")
CFLAGS += -DCMS_LEAK_CHECK
endif

CFLAGS += -Wall \
	-D_REENTRANT \
	-D_POSIX_C_SOURCE=200112L \
	-D_FILE_OFFSET_BITS=64 \
	-D_CMS_APP_USE_TIME_ \
	-I./

LINK = $(CC)

LDFLAGS = -lpthread \
	-ldl \
	-lrt \
	-ls2n \
	-L./lib/ \
	./lib/libssl.a \
	./lib/libcrypto.a \
	./lib/libev.a \
	./decode/libH264_5.a \
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
		$(wildcard static/*.cpp) \
		$(wildcard strategy/*.cpp) \
		$(wildcard taskmgr/*.cpp) \
		$(wildcard ts/*.cpp) \
		$(wildcard worker/*.cpp)


COBJS := $(CSRCS:.c=.o)
CPPOBJS := $(CPPSRCS:.cpp=.o)

TARGET = ./bin/cms
	
all : $(TARGET)

$(COBJS) : %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 
$(CPPOBJS) : %.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 
	
$(TARGET) : $(COBJS) $(CPPOBJS)
	@test -d './bin' || mkdir -p ./bin
	$(CC) -o $(TARGET) $(COBJS) $(CPPOBJS) $(LDFLAGS)

clean :
	rm -f $(COBJS) $(CPPOBJS) ./bin/cms

TARGETTYPE := APP
TARGETNAME := cms
