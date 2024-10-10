/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
package vendor.amlogic.hardware.vmx_webclient;

@VintfStability
enum VmxWebClientEventType {
    KEVENT_WEBCLIENT_VERSION = 0,
    KEVENT_UNIQUE_IDENTIFIER = 1,
    KEVENT_KEY_RETRIVAL_STATUS = 2,
    KEVENT_OUTPUT_CONTROL = 3,
    KEVENT_OPERATOR_DATA = 4,
}