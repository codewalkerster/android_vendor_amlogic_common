################################################################################
#
#  aml_hal_amdolby
#
################################################################################
LOCAL_PATH = $(shell pwd)
LDFLAGS += -Wl,--no-as-needed -lstdc++ -lpthread -ldl -lrt -L$(STAGING_DIR)/usr/lib
CFLAGS += -Wall -Werror -Wno-unknown-pragmas -Wno-format -Wno-format-security -Wno-error=unused-result \
          -O3 -fexceptions -fnon-call-exceptions -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include


SRCS  = $(LOCAL_PATH)/aml_hal_aml_dolby.cpp

INC_SRCS  = -I$(LOCAL_PATH)/../../../hal_inc
INC_SRCS += -I$(LOCAL_PATH)/../../adaptor/inc

# ---------------------------------------------------------------------
#  Build rules
BUILD_TARGETS = lib_hal_aml_dolby.so \

.PHONY: all install clean

lib_hal_aml_dolby.so: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC $(INC_SRCS) \
	-L$(LOCAL_PATH)/../../adaptor/src/aml_dolby -ladap_amdolby \
	-o $@ $^ $(LDLIBS)

all: $(BUILD_TARGETS)

clean:
	rm -f *.o $(BUILD_TARGETS)

install:
	install -m 0644 lib_hal_amdolby.so $(TARGET_DIR)/usr/lib

uninstall:
	rm -f $(TARGET_DIR)/usr/lib/lib_hal_aml_dolby.so
