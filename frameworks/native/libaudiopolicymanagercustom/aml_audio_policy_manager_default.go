package aml_audio_policy_manager

import (
    "os"
    "android/soong/android"
    "android/soong/cc"
)

func init() {
    android.RegisterModuleType("aml_audio_policy_manager_defaults", aml_audio_policy_manager_DefaultsFactory)
}

func aml_audio_policy_manager_Defaults(ctx android.LoadHookContext) {
    type propsE struct {
        Shared_libs  []string
        Static_libs  []string
        Cflags       []string
    }
    p := &propsE{}

    var buildType string
    curBuildType := os.Getenv("BOARD_COMPILE_ATV")
    if curBuildType == "false" {
        buildType = "-DAML_BOARD_COMPILE_ATV_TYPE=" + "0"
        p.Shared_libs = append(p.Shared_libs, "libaudiopolicycomponents")
    } else {
        buildType = "-DAML_BOARD_COMPILE_ATV_TYPE=" + "1"
        p.Static_libs = append(p.Static_libs, "libaudiopolicycomponents")
    }

    p.Cflags = append(p.Cflags, buildType)
    ctx.AppendProperties(p)
}

func aml_audio_policy_manager_DefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, aml_audio_policy_manager_Defaults)
    return module
}
