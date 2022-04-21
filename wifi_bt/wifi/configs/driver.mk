BUILD_MODULES := $(CONFIG_WIFI_MODULES)

define get-makefile-path
$(subst //,/,$($(1)_src_path)/$($(1)_build_path))
endef

define exist-makefile
$(shell if [ -f $(1)/Makefile -o -f $(1)/makefile ]; then echo "true"; else echo "false"; fi)
endef

build_drivers :=
$(foreach driver,\
 $(WIFI_SUPPORT_DRIVERS),\
 $(if $(filter true,$($(driver)_build)),\
  $(if $(filter true,$(call exist-makefile,$(ROOT_DIR)/$(call get-makefile-path,$(driver)))),\
   $(eval build_drivers += $(driver)),\
   $(warning "$(call get-makefile-path,$(driver))/Makefile" not found!)\
   )\
 )\
)

build_modules :=\
$(foreach driver,\
 $(build_drivers),\
 $($(driver)_modules))

ifeq ($(BUILD_MODULES), multiwifi)
BUILD_MODULES := $(build_modules)
endif

ifeq ($(BUILD_MODULES), )
BUILD_MODULES := $(build_modules)
else
support_modules :=
$(foreach module,\
 $(BUILD_MODULES),\
 $(if $(filter $(module),$(build_modules)),\
  $(eval support_modules += $(module)),\
  $(warning wifi module "$(module)" has no driver support!)\
 )\
)
BUILD_MODULES := $(support_modules)
endif

modules: $(addsuffix _modules,$(BUILD_MODULES))
	@echo "######build wifi drivers done!######"

modules_install: all_modules_install
	@echo "######install wifi modules done!######"

makeThreads ?= $(shell expr `cat /proc/cpuinfo |grep "physical id"|sort|uniq|wc -l` \* `cat /proc/cpuinfo \
 |grep "cpu cores"|uniq|wc -l` \* `cat /proc/cpuinfo |grep "processor"|wc -l`)

define direct_build_modules
$(strip $(1)_drv_modules):
	@echo "===>wifi: build driver->$(strip $(1))"
	mkdir -p $(patsubst %/,%,$(OUT_DIR)/$(K_REL_DIR)/$(strip $($(1)_src_path))/$(strip $($(1)_build_path)))
	+$(MAKE) -C $(patsubst %/,%,$(ROOT_DIR)/$(strip $($(1)_src_path))/$(strip $($(1)_build_path)))\
	 M=$(patsubst %/,%,$(K_REL_DIR)/$(strip $($(1)_src_path))/$(strip $($(1)_build_path)))\
	 $(MAKE_ARGS) $(strip $($(1)_args)) -j$(makeThreads)

$(strip $(1)_drv_modules_install):
	@echo "===>wifi: install modules->$(strip $(1))"
	make -C $(patsubst %/,%,$(ROOT_DIR)/$(strip $($(1)_src_path))/$(strip $($(1)_build_path)))\
	 M=$(patsubst %/,%,$(K_REL_DIR)/$(strip $($(1)_src_path))/$(strip $($(1)_build_path)))\
	 $(INSTALL_ARGS) $(MAKE_ARGS) $(strip $($(1)_args)) modules_install

$(addsuffix _modules,$(strip $($(1)_modules))): $(strip $(1)_drv_modules)
endef

define copy_build_modules
$(strip $(1)_drv_modules):
	@echo "===>wifi build driver->$(strip $(1))"
	mkdir -p $(strip $($(1)_copy_path))
	@echo Syncing directory $(ROOT_DIR)/$(strip $($(1)_src_path))/ to $(strip $($(1)_copy_path))
	rsync -a $(ROOT_DIR)/$(strip $($(1)_src_path))/ $(strip $($(1)_copy_path))
	+$(MAKE) -C $(patsubst %/,%,$(strip $($(1)_copy_path))/$(strip $($(1)_build_path)))\
	 M=$(patsubst %/,%,$(strip $($(1)_copy_path))/$(strip $($(1)_build_path)))\
	 $(MAKE_ARGS) $(strip $($(1)_args)) -j$(makeThreads)

$(strip $(1)_drv_modules_install):
	@echo "===>wifi: install modules->$(strip $(1))"
	make -C $(patsubst %/,%,$(strip $($(1)_copy_path))/$(strip $($(1)_build_path)))\
	 M=$(patsubst %/,%,$(strip $($(1)_copy_path))/$(strip $($(1)_build_path)))\
	 $(INSTALL_ARGS) $(MAKE_ARGS) $(strip $($(1)_args)) modules_install

$(addsuffix _modules,$(strip $($(1)_modules))): $(strip $(1)_drv_modules)
endef

direct_build_drivers :=\
$(foreach driver,\
 $(build_drivers),\
 $(if $($(driver)_copy_path),,$(driver)))

copy_build_drivers :=\
$(foreach driver,\
 $(build_drivers),\
 $(if $($(driver)_copy_path),$(driver)))

install_drivers :=\
$(sort \
 $(foreach module,\
  $(BUILD_MODULES),\
   $(foreach driver,\
    $(build_drivers),\
     $(if $(filter $(module),$($(driver)_modules)),$(driver))\
   )\
 )\
)

$(foreach driver,\
 $(direct_build_drivers),\
 $(eval $(call direct_build_modules,$(driver)))\
)

$(foreach driver,\
 $(copy_build_drivers),\
 $(eval $(call copy_build_modules,$(driver)))\
)

all_modules_install:
	@for driver in $(install_drivers); do\
	 make $(strip "$$driver"_drv_modules_install);\
	done

