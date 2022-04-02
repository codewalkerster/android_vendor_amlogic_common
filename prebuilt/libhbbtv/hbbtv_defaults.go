package amlogic_hbbtv

import (
    // "fmt"
    "android/soong/android"
    "android/soong/cc"
    "github.com/google/blueprint/proptools"
)

func init() {
    android.RegisterModuleType("amlogic_hbbtv_go_defaults",hbbtv_DefaultsFactory)
}

func hbbtv_DefaultsFactory() (android.Module) {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, hbbtv_Defaults)
    return module
}

func hbbtv_Defaults(ctx android.LoadHookContext) {
    type props struct {
        Enabled *bool
    }
    p := &props{}
    p.Enabled = hbbtv_globalDefaults(ctx)
    ctx.AppendProperties(p)
}

func hbbtv_globalDefaults(ctx android.BaseContext) (*bool) {
    var module_enabled *bool
    if android.ExistentPathForSource(ctx, "vendor/amlogic/common/prebuilt/hbbtv").Valid() == true {
        module_enabled = proptools.BoolPtr(false)
        // fmt.Println("avcodec:vendor/amlogic/common/AmFFmpegAdapter exist, use source to build")
    } else {
        // fmt.Println("avcodec:vendor/amlogic/common/AmFFmpegAdapter not exist, use prebuilt codec lib")
    }
    return module_enabled
}
