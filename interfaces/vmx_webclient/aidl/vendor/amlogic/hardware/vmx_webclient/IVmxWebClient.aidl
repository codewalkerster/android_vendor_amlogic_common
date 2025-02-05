/*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

package vendor.amlogic.hardware.vmx_webclient;

import vendor.amlogic.hardware.vmx_webclient.VmxWebClientDecryptParam;
import vendor.amlogic.hardware.vmx_webclient.Status;
import vendor.amlogic.hardware.vmx_webclient.IVmxWebClientCallback;
import vendor.amlogic.hardware.vmx_webclient.KeyRequestParam;
import vendor.amlogic.hardware.vmx_webclient.PipelineParam;

@VintfStability
interface IVmxWebClient {
    int createInstance();

    int decrypt(in byte[] sessionid, in byte[] keyid, in byte[] keyurl, in byte[] indata, in byte[] iv, out byte[] outdata);

    int decryptSecure(in VmxWebClientDecryptParam para);

    int destroyInstance();

    void setCallback(in byte[] sessionId, in IVmxWebClientCallback callback);

    void getProperty(in String prop, out byte[] value);

    void setProperty(in String prop, in byte[] value);

    int getCdmErr();

    int fetchKey(in byte[] sessionId, in KeyRequestParam para);

    int provision(in byte[] request);

    boolean isProvisioned();

    byte[] createPipeline(in PipelineParam para);

    int destroyPipeline(in byte[] engineId);
}
