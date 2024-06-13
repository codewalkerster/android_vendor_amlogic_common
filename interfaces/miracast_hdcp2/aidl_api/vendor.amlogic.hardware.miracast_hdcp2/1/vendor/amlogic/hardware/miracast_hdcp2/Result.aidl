 /**
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

package vendor.amlogic.hardware.miracast_hdcp2;

@VintfStability
@Backing(type="int")
enum Result {
    OBSERVER_NOTIFY = 0x00000001,
    HDCP_SET_OBSERVER = 0x00000003,
    HDCP_INIT_ASYNC = 0x00000004,
    HDCP_SHUTDOWN_ASYNC = 0x00000005,
    HDCP_GET_CAPS = 0x00000006,
    HDCP_ENCRYPT = 0x00000007,
    HDCP_ENCRYPT_NATIVE = 0x00000008,
    HDCP_DECRYPT = 0x00000009,
}
