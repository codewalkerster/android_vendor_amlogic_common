package tokenmanager

import (
    "android/soong/android"
    "android/soong/cc"
    "github.com/google/blueprint/proptools"
    //"fmt"
)

func init() {
    android.RegisterModuleType("tokenmanager_defaults", TokenManagerDefaultsFactory)
}

func TokenManagerDefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, func(ctx android.LoadHookContext) {
        type props struct {
            Enabled *bool
        }
        p := &props{}

        vconfig := ctx.Config().VendorConfig("amlogic_vendorconfig")
        if vconfig.Bool("netflix_mgkid") == false {
            p.Enabled = proptools.BoolPtr(false)
        }
        ctx.AppendProperties(p)
    })
    return module
}

