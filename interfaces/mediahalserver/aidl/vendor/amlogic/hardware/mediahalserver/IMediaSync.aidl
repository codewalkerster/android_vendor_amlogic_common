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

import vendor.amlogic.hardware.mediahalserver.Result;
import vendor.amlogic.hardware.mediahalserver.MediaTimeType;
import vendor.amlogic.hardware.mediahalserver.TimeUnit;

@VintfStability
interface IMediaSync {

    /**
     * bind mediasync to specified avSyncHwId
     *
     * @param avSyncHwId
     *
     * @return Result status of the operation.
     *         OK if successful,
     *         FAIL if failed for other reasons.
     */
    Result bindInstanceId(in int avSyncHwId);

    /**
     * get media time of mediasync
     *
     * @param in mediaTimeType the MediaTimeType for the media time
     * @param in timeunit the TimeUnit for the media time
     *
     * @return Result status of the operation.
     *         OK if successful,
     *         FAIL if failed for other reasons.
     */
    Result getMediaTime(in MediaTimeType mediaTimeType, in TimeUnit timeunit, out long[] time);

    /**
     * destroy mediasync
     *
     * @return Result status of the operation.
     *         OK if successful,
     *         FAIL if failed for other reasons.
     */
    Result destroy();
}
