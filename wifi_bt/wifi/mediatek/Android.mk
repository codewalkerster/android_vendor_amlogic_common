ifneq ($(BOARD_USE_ODM_WIFI_BT),true)
ifeq ($(BOARD_WIFI_VENDOR), mtk)
    include $(call all-subdir-makefiles)
endif
endif
