SOONG_CONFIG_NAMESPACES += mdns
SOONG_CONFIG_mdns += \
    board

ifeq ($(TARGET_BUILD_MDNS),false)
SOONG_CONFIG_mdns_board := mdns_disable
else
ifneq ($(wildcard device/google/atv/MdnsOffloadManagerService),)
PRODUCT_PACKAGES += \
    MdnsOffloadCmdService \
    MdnsOffloadManagerService \
    MdnsOffloadManagerServiceOverlay \
    DroidMdnsOffloadService \
    droidmdnsoffload-service

DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE += vendor/amlogic/common/frameworks/services/droidmdnsoffload/device_framework_matrix.xml
SOONG_CONFIG_mdns_board := mdns_enable
else
SOONG_CONFIG_mdns_board := mdns_disable
endif
endif