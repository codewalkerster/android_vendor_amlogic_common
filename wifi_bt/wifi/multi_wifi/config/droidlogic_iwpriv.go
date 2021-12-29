package droidlogic_iwpriv

import (
     // "fmt"
     "android/soong/android"
     "android/soong/cc"
     "github.com/google/blueprint/proptools"
)

func init() {
     // fmt.Println("init: droidlogic_iwpriv")
     android.RegisterModuleType("iwpriv_go_defaults", iwpriv_go_DefaultsFactory)
}

func iwpriv_go_DefaultsFactory() (android.Module) {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, func(ctx android.LoadHookContext) {
         type props struct {
             Enabled *bool
         }
         p := &props{}

         if android.ExistentPathForSource(ctx, "vendor/google/tools/iwpriv").Valid() == true {
             p.Enabled = proptools.BoolPtr(false)
             // fmt.Println("iwpriv: vendor/google/tools/iwpriv exist, use google to build")
         } else {
             // fmt.Println("iwpriv: vendor/google/tools/iwpriv not exist, use amlogic to build")
         }
         ctx.AppendProperties(p)
	})
    return module
}
