package aml_audio_policy_manager_engine

import (
    "android/soong/android"
    "android/soong/cc"
)

func init() {
    android.RegisterModuleType("aml_audio_policy_manager_engine_defaults", aml_audio_policy_manager_engine_DefaultsFactory)
}

func aml_audio_policy_manager_engine_Defaults(ctx android.LoadHookContext) {
    type propsE struct {
        Shared_libs  []string
        Static_libs  []string
        Cflags       []string
    }
    p := &propsE{}

    vconfig := ctx.Config().VendorConfig("amlogic_vendorconfig")
    board_compile_ver := vconfig.String("board_compile_version")
    if board_compile_ver == "aosp" {
        p.Cflags = append(p.Cflags, "-DAML_BOARD_COMPILE_AOSP_TYPE")
        p.Shared_libs = append(p.Shared_libs, "libaudiopolicycomponents")
        p.Shared_libs = append(p.Shared_libs, "libaudio_aidl_conversion_common_cpp")
    } else {
        p.Static_libs = append(p.Static_libs, "libaudiopolicycomponents")
    }
    ctx.AppendProperties(p)
}

func aml_audio_policy_manager_engine_DefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, aml_audio_policy_manager_engine_Defaults)
    return module
}
