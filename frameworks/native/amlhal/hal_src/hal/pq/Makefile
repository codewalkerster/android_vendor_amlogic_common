################################################################################
#
#  aml_hal_pq
#
################################################################################
LOCAL_PATH = $(shell pwd)
LDFLAGS += -Wl,--no-as-needed -lstdc++ -lpthread -ldl -lrt -L$(STAGING_DIR)/usr/lib
CFLAGS += -Wall -Werror -Wno-unknown-pragmas -Wno-format -Wno-format-security -Wno-error=unused-result \
          -O3 -fexceptions -fnon-call-exceptions -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include


SRCS  = $(LOCAL_PATH)/aml_hal_pq.cpp
SRCS += $(LOCAL_PATH)/aml_hal_di.cpp
SRCS += $(LOCAL_PATH)/aml_hal_ld.cpp
SRCS += $(LOCAL_PATH)/aml_hal_memc.cpp

INC_SRCS  = -I$(LOCAL_PATH)/../../../hal_inc
INC_SRCS += -I$(LOCAL_PATH)/../../adaptor/inc
INC_SRCS += -I$(LOCAL_PATH)/../../lib/pq/LibTableOSD
INC_SRCS += -I$(LOCAL_PATH)/../../lib/pq/LibTablePQ


# ---------------------------------------------------------------------
#  Build rules

BUILD_TARGETS = lib_hal_pq.so \

.PHONY: all install clean

lib_hal_pq.so: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC $(INC_SRCS) \
	-L$(LOCAL_PATH)/../../adaptor/src/pq -ladap_pq \
	-L$(LOCAL_PATH)/../../lib/pq/LibTableOSD -l_PQTableOSD \
	-L$(LOCAL_PATH)/../../lib/pq/LibTablePQ -l_PQTable \
	-o $@ $^ $(LDLIBS)


all: $(BUILD_TARGETS)

clean:
	rm -f *.o $(BUILD_TARGETS)

install:
	install -m 0644 lib_hal_pq.so $(TARGET_DIR)/usr/lib

uninstall:
	rm -f $(TARGET_DIR)/usr/lib/lib_hal_pq.so
