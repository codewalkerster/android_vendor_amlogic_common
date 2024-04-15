package tdk

import (
    "android/soong/android"
    "android/soong/cc"
)

func init() {
    android.RegisterModuleType("soundtrigger_go_defaults", soundtrigger_go_DefaultsFactory)
}

func soundtrigger_go_DefaultsFactory() (android.Module) {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, func(ctx android.LoadHookContext) {
        type props struct {
            Srcs []string
            Shared_libs []string
            Init_rc []string
            Vintf_fragments []string
        }

        p := &props{}
        vconfig := ctx.Config().VendorConfig("amlogic_vendorconfig")
        if (vconfig.Bool("enable_lowpower_hotword") == true) {
            p.Shared_libs = append(p.Shared_libs, "android.hardware.soundtrigger@2.0")
            p.Shared_libs = append(p.Shared_libs, "android.hardware.soundtrigger@2.0-core")
            p.Shared_libs = append(p.Shared_libs, "android.hardware.soundtrigger@2.1")
            p.Shared_libs = append(p.Shared_libs, "android.hardware.soundtrigger@2.2")
            p.Shared_libs = append(p.Shared_libs, "android.hardware.soundtrigger@2.3")
            p.Shared_libs = append(p.Shared_libs, "android.hidl.allocator@1.0")
            p.Shared_libs = append(p.Shared_libs, "android.hidl.memory@1.0")
            p.Shared_libs = append(p.Shared_libs, "libhidlmemory")
            p.Vintf_fragments = append(p.Vintf_fragments, "soundtrigger/android.hardware.soundtringger@2.3-service.droidlogic.xml")
            p.Init_rc = []string{"android.hardware.audio.soundtrigger.service-droidlogic.rc"}
            p.Srcs = append(p.Srcs, "soundtrigger/SoundTriggerHw.cpp")
        } else {
            p.Init_rc = []string{"android.hardware.audio.service-droidlogic.rc"}
        }
        ctx.AppendProperties(p)
    })
    return module
}

