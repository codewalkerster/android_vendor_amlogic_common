/*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
package vendor.amlogic.hardware.vmx_webclient;

import android.hardware.common.NativeHandle;
import vendor.amlogic.hardware.vmx_webclient.Mode;
import vendor.amlogic.hardware.vmx_webclient.Pattern;
import vendor.amlogic.hardware.vmx_webclient.SubSample;
import vendor.amlogic.hardware.vmx_webclient.StreamingFormat;
import vendor.amlogic.hardware.vmx_webclient.MethodInfo;

/**
 * VmxWebClientDecryptParam describes a decrypt
 */

@VintfStability
parcelable VmxWebClientDecryptParam {
    int secure;
    int streamingFormat;
    int methodInfo;
    int keySeq;
    byte[] keyId;
    byte[] keyUrl;
    byte[] iv;
    Mode mode;
    Pattern pattern;
    SubSample[] subSamples;
    byte[] src;
    byte[] dst;
    NativeHandle sourceDesc;
    NativeHandle destDesc;
    long srcOffset;
    long offset;
    long destOffset;
    int engineId;
}
