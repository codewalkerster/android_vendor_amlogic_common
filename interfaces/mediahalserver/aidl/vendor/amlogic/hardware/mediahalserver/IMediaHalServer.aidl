/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package vendor.amlogic.hardware.mediahalserver;

import android.hardware.common.fmq.MQDescriptor;
import android.hardware.common.fmq.SynchronizedReadWrite;

import vendor.amlogic.hardware.mediahalserver.IMediaSync;
import vendor.amlogic.hardware.mediahalserver.Result;

@VintfStability
interface IMediaHalServer {
    /**
     * open a session
     *
     * @param out sessionId, sessionId is unique id to specify
     *         the client. We support multi client here.
     *
     * @return Result status of the operation.
     *         OK if successful,
     *         FAIL if failed for other reasons.
     */
    Result openSession(out int[] sessionId);

    /**
     * close a session
     *
     * @param sessionId the sessionId to close
     *
     * @param Result status of the operation.
     *         OK if successful,
     *         FAIL if failed for other reasons.
     */
    Result closeSession(in int sessionId);

    /**
     * create mediasync for specified sessionId
     *
     * @param sessionId the sessionId to create mediasync
     *
     * @return the created MediaSync.
     */
    IMediaSync createMediaSync(in int sessionId);
}
