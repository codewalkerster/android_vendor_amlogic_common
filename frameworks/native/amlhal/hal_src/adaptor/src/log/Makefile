################################################################################
#
#  log_cout
#
################################################################################
LOCAL_PATH = $(shell pwd)
LDFLAGS += -Wl,--no-as-needed -lstdc++ -lpthread -ldl -lrt -L$(STAGING_DIR)/usr/lib
CFLAGS += -Wall -Werror -Wno-unknown-pragmas -Wno-format -Wno-format-security -Wno-error=unused-result \
          -O3 -fexceptions -fnon-call-exceptions -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include


adap_log_SRCS  = \
	$(LOCAL_PATH)/log_cout.cpp \
	$(NULL)

adap_HEADERS = \
	$(LOCAL_PATH)/../../inc/log \
	$(NULL)

# ---------------------------------------------------------------------
#  Build rules

BUILD_TARGETS = liblog_cout.so \

.PHONY: all install clean

liblog_cout.so: $(adap_log_SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -I$(adap_HEADERS) \
	-o $@ $^ $(LDLIBS)

all: $(BUILD_TARGETS)

clean:
	rm -f *.o $(BUILD_TARGETS)

install:
	install -m 0644 liblog_cout.so $(TARGET_DIR)/usr/lib

uninstall:
	rm -f $(TARGET_DIR)/usr/lib/liblog_cout.so
