/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

package vendor.amlogic.hardware.vmx_webclient;

import vendor.amlogic.hardware.vmx_webclient.VmxWebClientEventType;

@VintfStability
interface IVmxWebClientCallback {
   oneway void sendEvent(in VmxWebClientEventType eventType, in byte[] sessionId, in byte[] event);
}