####################################################################################
# define variables
######################################################
LOCAL_ROOT_DIR             := $(ROOT_DIR)
LOCAL_OUT_DIR              := $(OUT_DIR)
LOCAL_KERNEL_TO_ROOT_PATH  := $(KERNEL_TO_ROOT_PATH)
LOCAL_CONFIG_BUILD_MODULES := $(CONFIG_WIFI_MODULES)
LOCAL_WIFI_SUPPORT_DRIVERS := $(WIFI_SUPPORT_DRIVERS)
LOCAL_MAKE_ARGS            := $(MAKE_ARGS)
LOCAL_INSTALL_ARGS         := $(INSTALL_ARGS)
LOCAL_WIFI_DRV_MAKE_JOBS   := $(WIFI_DRV_MAKE_JOBS)
######################################################

####################################################################################
# define functions
######################################################
define get-make-threads
$(strip \
 $(if $(LOCAL_WIFI_DRV_MAKE_JOBS),$(LOCAL_WIFI_DRV_MAKE_JOBS),\
  $(shell expr `cat /proc/cpuinfo |grep "physical id"|sort|uniq|wc -l` \* `cat /proc/cpuinfo \
   |grep "cpu cores"|uniq|wc -l` \* `cat /proc/cpuinfo |grep "processor"|wc -l`))\
)
endef

define get-drv-src-path
$(strip $($(1)_src_path))
endef

define get-drv-copy-path
$(strip $($(1)_copy_path))
endef

define get-drv-makefile-path
$(strip $(patsubst %/,%,$(call get-drv-src-path,$(1))/$(strip $($(1)_build_path))))
endef

define get-drv-build-path
$(strip \
 $(if $($(1)_copy_path),\
  $(patsubst %/,%,$(call get-drv-copy-path,$(1))/$(strip $($(1)_build_path))),\
  $(patsubst %/,%,$(call get-drv-src-path,$(1))/$(strip $($(1)_build_path))))\
)
endef

define get-drv-build-args
$(strip $($(1)_args))
endef

define drv-is-existed
$(shell if [ -f $(1)/Makefile -o -f $(1)/makefile ]; then echo "true"; else echo "false"; fi)
endef

define drv-has-modules-install
$(strip \
 $(eval f := $(shell if [ -f $(1)/Makefile ]; then echo "$(1)/Makefile"; else echo "$(1)/makefile"; fi))\
 $(shell if [ x$(shell cat $(f) | grep -w "modules_install:") = x"modules_install:" ];then echo "true"; else echo "false"; fi)\
)
endef

define get-wifi-drivers
$(strip \
 $(foreach driver,\
  $(LOCAL_WIFI_SUPPORT_DRIVERS),\
  $(if $(filter true,$($(driver)_build)),\
   $(if $(filter true,$(call drv-is-existed,$(LOCAL_ROOT_DIR)/$(call get-drv-makefile-path,$(driver)))),$(driver))))\
)
endef

define get-wifi-modules
$(strip \
 $(eval all_modules := $(foreach driver,$(call get-wifi-drivers),$($(driver)_modules)))\
 $(eval config_modules := $(foreach module,$(LOCAL_CONFIG_BUILD_MODULES),$(if $(filter $(module),$(all_modules)),$(module))))\
 $(if $(LOCAL_CONFIG_BUILD_MODULES),\
  $(if $(filter multiwifi,$(LOCAL_CONFIG_BUILD_MODULES)),$(all_modules),$(config_modules)),\
  $(all_modules))\
)
endef

define get-install-drivers
$(strip \
$(eval install_drivers := $(strip $(sort $(foreach module,$(call get-wifi-modules),\
 $(foreach driver,$(call get-wifi-drivers),$(if $(filter $(module),$($(driver)_modules)),$(driver)))))))\
$(foreach driver,$(install_drivers),\
 $(if $(filter true,$(call drv-has-modules-install,$(LOCAL_ROOT_DIR)/$(call get-drv-makefile-path,$(driver)))),$(driver)))\
)
endef

define print-ignored-drivers
$(foreach driver,\
 $(LOCAL_WIFI_SUPPORT_DRIVERS),\
 $(if $(filter true,$($(driver)_build)),\
  $(if $(filter false,$(call drv-is-existed,$(LOCAL_ROOT_DIR)/$(call get-drv-makefile-path,$(driver)))),\
   $(warning ignore build wifi driver "$(driver)"! because "$(call get-drv-makefile-path,$(driver))/Makefile" not found!)))\
)
endef

define print-ignored-modules
$(eval all_modules := $(foreach driver,$(call get-wifi-drivers),$($(driver)_modules)))\
$(if $(LOCAL_CONFIG_BUILD_MODULES),\
 $(if $(filter multiwifi,$(LOCAL_CONFIG_BUILD_MODULES)),,\
  $(foreach module,$(LOCAL_CONFIG_BUILD_MODULES),\
   $(if $(filter $(module),$(all_modules)),,\
    $(warning ignore wifi module "$(module)"! because "$(module)" has no driver support!))))\
)
endef

define def-direct-build-driver-cmd
$(strip $(1)_drv_modules):
	@echo "===>wifi: build driver $(strip $(1))"
	mkdir -p $(LOCAL_OUT_DIR)/$(LOCAL_KERNEL_TO_ROOT_PATH)/$(call get-drv-build-path,$(1))
	+$(MAKE) -C $(LOCAL_ROOT_DIR)/$(call get-drv-build-path,$(1)) \
	 M=$(LOCAL_KERNEL_TO_ROOT_PATH)/$(call get-drv-build-path,$(1)) \
	 $(LOCAL_MAKE_ARGS) $(call get-drv-build-args,$(1)) -j$(call get-make-threads)

$(strip $(1)_drv_modules_install):
	@echo "===>wifi: driver $(strip $(1)) modules_install"
	make -C $(LOCAL_ROOT_DIR)/$(call get-drv-build-path,$(1)) \
	 M=$(LOCAL_KERNEL_TO_ROOT_PATH)/$(call get-drv-build-path,$(1)) \
	 $(LOCAL_INSTALL_ARGS) $(LOCAL_MAKE_ARGS) $(call get-drv-build-args,$(1)) modules_install

$(addsuffix _modules,$(strip $($(1)_modules))): $(strip $(1)_drv_modules)
endef

define def-copy-build-driver-cmd
$(strip $(1)_drv_modules):
	@echo "===>wifi: build driver $(strip $(1))"
	mkdir -p $(call get-drv-copy-path,$(1))
	@echo Syncing directory $(LOCAL_ROOT_DIR)/$(call get-drv-src-path,$(1))/ to $(call get-drv-copy-path,$(1))
	rsync -a $(LOCAL_ROOT_DIR)/$(call get-drv-src-path,$(1))/ $(call get-drv-copy-path,$(1))
	+$(MAKE) -C $(call get-drv-build-path,$(1)) \
	 M=$(call get-drv-build-path,$(1)) \
	 $(LOCAL_MAKE_ARGS) $(call get-drv-build-args,$(1)) -j$(call get-make-threads)

$(strip $(1)_drv_modules_install):
	@echo "===>wifi: driver $(strip $(1)) modules_install"
	make -C $(call get-drv-build-path,$(1)) \
	 M=$(call get-drv-build-path,$(1)) \
	 $(LOCAL_INSTALL_ARGS) $(LOCAL_MAKE_ARGS) $(call get-drv-build-args,$(1)) modules_install

$(addsuffix _modules,$(strip $($(1)_modules))): $(strip $(1)_drv_modules)
endef

define def-build-driver-rules
$(foreach driver,$(call get-wifi-drivers),\
 $(if $($(driver)_copy_path),\
  $(eval $(call def-copy-build-driver-cmd,$(driver))),\
  $(eval $(call def-direct-build-driver-cmd,$(driver))))\
)
endef
######################################################

####################################################################################
# build rules
######################################################
$(eval $(call print-ignored-drivers))

$(eval $(call print-ignored-modules))

modules: $(addsuffix _modules,$(call get-wifi-modules))
	@echo "######build wifi drivers done!######"

modules_install: all_modules_install
	@echo "######wifi drivers modules_install done!######"

all_modules_install:
	@for driver in $(call get-install-drivers); do\
	 make $(strip "$$driver"_drv_modules_install);\
	done

$(eval $(call def-build-driver-rules))
######################################################