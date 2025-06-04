/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

package vendor.amlogic.hardware.droidaudio;

@VintfStability
interface IDroidAudioClient {
    oneway void onDroidAudioEvent(int event, in int[] data);
}
