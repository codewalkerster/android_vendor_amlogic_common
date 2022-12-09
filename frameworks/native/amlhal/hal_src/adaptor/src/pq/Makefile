################################################################################
#
#  adap_pq
#
################################################################################
LOCAL_PATH = $(shell pwd)
LDFLAGS += -Wl,--no-as-needed -lstdc++ -lpthread -ldl -lrt -L$(STAGING_DIR)/usr/lib
CFLAGS += -Wall -Werror -Wno-unknown-pragmas -Wno-format -Wno-format-security -Wno-error=unused-result \
          -O3 -fexceptions -fnon-call-exceptions -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include


adap_pq_SRCS  = \
	$(LOCAL_PATH)/adap_pq.cpp \
	$(LOCAL_PATH)/adap_di.cpp \
	$(LOCAL_PATH)/adap_ld.cpp \
	$(LOCAL_PATH)/adap_memc.cpp	\
	$(NULL)

adap_HEADERS = \
	-I$(LOCAL_PATH)/../../inc \
	-I$(LOCAL_PATH)/../../../lib/pq/LibTableOSD \
	-I$(LOCAL_PATH)/../../../lib/pq/LibTablePQ \
	$(NULL)

# ---------------------------------------------------------------------
#  Build rules
BUILD_TARGETS = libadap_pq.so \

.PHONY: all install clean

libadap_pq.so: $(adap_pq_SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC $(adap_HEADERS) \
	-L$(LOCAL_PATH)/../log -llog_cout \
	-L$(LOCAL_PATH)/../../../lib/pq/LibTableOSD -l_PQTableOSD \
	-L$(LOCAL_PATH)/../../../lib/pq/LibTablePQ -l_PQTable \
	-o $@ $^ $(LDLIBS)

all: $(BUILD_TARGETS)

clean:
	rm -f *.o $(BUILD_TARGETS)

install:
	install -m 0644 libadap_pq.so $(TARGET_DIR)/usr/lib

uninstall:
	rm -f $(TARGET_DIR)/usr/lib/libadap_pq.so
