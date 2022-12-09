#################################################################################
#
#  lib_PQTable
#
#################################################################################
-include curFile

LOCAL_PATH = $(shell pwd)
LDFLAGS += -Wl,--no-as-needed -lstdc++ -lpthread -ldl -lrt -L$(STAGING_DIR)/usr/lib
CFLAGS += -Wall -Werror -Wno-unknown-pragmas -Wno-format -Wno-format-security -Wno-error=unused-result \
          -O3 -fexceptions -fnon-call-exceptions -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include

SRCS  = $(LOCAL_PATH)/PQTable.cpp
SRCS += $(LOCAL_PATH)/PQTableLoader.cpp
SRCS += $(LOCAL_PATH)/AML_PQ_Table_Public.cpp
SRCS += $(LOCAL_PATH)/PQTableLogic.cpp

GENSRC = ./GenerateBinFile.cpp
GENSRC += ./PQTableLoader.cpp

BUILD_TARGETS = lib_PQTable.so \

.PHONY: all install clean

lib_PQTable.so: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC $(INC_SRCS) \
	-o $@ $^ $(LDLIBS)

all: $(BUILD_TARGETS)

clean:
	rm -f *.o $(BUILD_TARGETS)

GENclean:
	rm -f *.o

install:
	install -m 0644 lib_PQTable.so $(TARGET_DIR)/usr/lib

uninstall:
	rm -f $(TARGET_DIR)/usr/lib/lib_PQTable.so

FilePQ: GENclean
	g++ -g -Wall -ansi -o GenPQFile $(GENSRC) $(CUR_PQFILE)
	@echo ===== GenPQFile PQ BIN generated !! =====