ifneq ($(BOARD_USE_ODM_WIFI_BT),true)
ifeq ($(BOARD_WIFI_VENDOR), realtek)
	include $(call all-subdir-makefiles)
endif
endif
