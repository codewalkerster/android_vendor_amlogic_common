################################################################################
#
# lib_PQTableOSD
#
################################################################################
-include curFile

LOCAL_PATH = $(shell pwd)
LDFLAGS += -Wl,--no-as-needed -lstdc++ -lpthread -ldl -lrt -L$(STAGING_DIR)/usr/lib
CFLAGS += -Wall -Werror -Wno-unknown-pragmas -Wno-format -Wno-format-security -Wno-error=unused-result \
          -O3 -fexceptions -fnon-call-exceptions -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include

SRCS  = $(LOCAL_PATH)/PQTableOSD.cpp
SRCS += $(LOCAL_PATH)/PQTableLoaderOSD.cpp
SRCS += $(LOCAL_PATH)/AML_PQ_OSD_Table_Public.cpp


GENSRC = ./GenerateBinFileOSD.cpp
GENSRC += ./PQTableLoaderOSD.cpp


BUILD_TARGETS = lib_PQTableOSD.so \

.PHONY: all install clean

lib_PQTableOSD.so: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC $(INC_SRCS) \
	-o $@ $^ $(LDLIBS)

all: $(BUILD_TARGETS)

clean:
	rm -f *.o $(BUILD_TARGETS)

GENclean:
	rm -f *.o

install:
	install -m 0644 lib_PQTableOSD.so $(TARGET_DIR)/usr/lib

uninstall:
	rm -f $(TARGET_DIR)/usr/lib/lib_PQTableOSD.so

FilePQOSD: GENclean
	g++ -g -Wall -ansi -o GenPQOsdFile $(GENSRC) $(CUR_PQOSDFILE)
	@echo ===== GenPQOsdFile PQ OSD BIN generated !! =====