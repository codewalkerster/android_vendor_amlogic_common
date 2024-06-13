/** 
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

package vendor.amlogic.hardware.miracast_hdcp2;

import vendor.amlogic.hardware.miracast_hdcp2.IHDCPObserver;
import vendor.amlogic.hardware.miracast_hdcp2.EncryptResult;

@VintfStability
interface IHDCP {
    byte[] decrypt(in byte[] inData, in int streamCTR,
        in long outInputCTR, in int outAddr);

    byte[] decryptSecure(in byte[] decryptInfo, in byte[] inData);

    EncryptResult encrypt(in byte[] inData, in int streamCTR);

    int getCaps();

    void initAsync(in String host, in int port);

    void setObserver(in IHDCPObserver observer);

    void shutdownAsync();
}
