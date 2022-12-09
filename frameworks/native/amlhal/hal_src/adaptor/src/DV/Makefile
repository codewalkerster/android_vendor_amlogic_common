################################################################################
#
#  adap_dv
#
################################################################################
LOCAL_PATH = $(shell pwd)
LDFLAGS += -Wl,--no-as-needed -lstdc++ -lpthread -ldl -lrt -L$(STAGING_DIR)/usr/lib
CFLAGS += -Wall -Werror -Wno-unknown-pragmas -Wno-format -Wno-format-security -Wno-error=unused-result \
          -O3 -fexceptions -fnon-call-exceptions -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include


adap_display_SRCS  = \
	$(LOCAL_PATH)/adap_dv.cpp \
	$(NULL)

adap_HEADERS = \
	$(LOCAL_PATH)/../../inc \
	$(NULL)

# ---------------------------------------------------------------------
#  Build rules
BUILD_TARGETS = libadap_dv.so \

.PHONY: all install clean

libadap_dv.so: $(adap_pq_SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -I$(adap_HEADERS) \
	-L$(LOCAL_PATH)/../log -llog_cout \
	-o $@ $^ $(LDLIBS)

all: $(BUILD_TARGETS)

clean:
	rm -f *.o $(BUILD_TARGETS)

install:
	install -m 0644 libadap_dv.so $(TARGET_DIR)/usr/lib

uninstall:
	rm -f $(TARGET_DIR)/usr/lib/libadap_dv.so
