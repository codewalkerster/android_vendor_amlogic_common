################################################################################
#
#  aml_hal_disp
#
################################################################################
LOCAL_PATH = $(shell pwd)
LDFLAGS += -Wl,--no-as-needed -lstdc++ -lpthread -ldl -lrt -L$(STAGING_DIR)/usr/lib
CFLAGS += -Wall -Werror -Wno-unknown-pragmas -Wno-format -Wno-format-security -Wno-error=unused-result \
          -O3 -fexceptions -fnon-call-exceptions -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include


hal_display_SRCS  = \
	$(LOCAL_PATH)/aml_hal_display.cpp \
	$(NULL)

hal_HEADERS = \
	$(LOCAL_PATH)/../../../hal_inc \
	$(NULL)

# ---------------------------------------------------------------------
#  Build rules
BUILD_TARGETS = lib_hal_display.so \

.PHONY: all install clean

lib_hal_display.so: $(hal_display_SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -I$(hal_HEADERS) \
	-L$(LOCAL_PATH)/../../adaptor/src/display -ladap_display \
	-o $@ $^ $(LDLIBS)

all: $(BUILD_TARGETS)

clean:
	rm -f *.o $(BUILD_TARGETS)

install:
	install -m 0644 lib_hal_display.so $(TARGET_DIR)/usr/lib

uninstall:
	rm -f $(TARGET_DIR)/usr/lib/lib_hal_display.so
