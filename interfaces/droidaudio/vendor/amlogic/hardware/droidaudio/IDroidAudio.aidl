/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

package vendor.amlogic.hardware.droidaudio;

import vendor.amlogic.hardware.droidaudio.Status;
import vendor.amlogic.hardware.droidaudio.IDroidAudioClient;

@VintfStability
interface IDroidAudio {
    int init();
    int reset();
    int registerClient(in IDroidAudioClient client);
    int setAudioCmdParam(int cmd, int param1, int param2, int param3);
    int setOutputDevices(in int[] devices);
    int[] getOutputDevices();
    int setCoexistSpdifOther(boolean enable);
    int setMusicStreamVolume(int index);
}
