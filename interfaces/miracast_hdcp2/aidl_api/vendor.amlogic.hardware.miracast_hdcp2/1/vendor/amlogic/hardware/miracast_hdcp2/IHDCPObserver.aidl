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
interface IHDCPObserver {
    void notify(in int msg, in int ext1, in int ext2);
}
