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
parcelable EncryptResult {
    long outInputCTR;
    byte[] outData;
    boolean secure;
    long handle;
    long reserved;
    long reserved_1;
}

