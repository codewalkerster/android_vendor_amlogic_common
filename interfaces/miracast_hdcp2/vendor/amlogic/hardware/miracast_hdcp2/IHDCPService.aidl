/** 
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

package vendor.amlogic.hardware.miracast_hdcp2;

import vendor.amlogic.hardware.miracast_hdcp2.IHDCP;

@VintfStability
interface IHDCPService {
    IHDCP makeHDCP(in boolean createEncryptionModule);
}
