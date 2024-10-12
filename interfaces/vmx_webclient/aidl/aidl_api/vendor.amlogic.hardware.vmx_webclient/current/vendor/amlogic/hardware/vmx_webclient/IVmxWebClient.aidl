/*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
///////////////////////////////////////////////////////////////////////////////
// THIS FILE IS IMMUTABLE. DO NOT EDIT IN ANY CASE.                          //
///////////////////////////////////////////////////////////////////////////////

// This file is a snapshot of an AIDL file. Do not edit it manually. There are
// two cases:
// 1). this is a frozen version file - do not edit this in any case.
// 2). this is a 'current' file. If you make a backwards compatible change to
//     the interface (from the latest frozen version), the build system will
//     prompt you to update this file with `m <name>-update-api`.
//
// You must not make a backward incompatible change to any AIDL file built
// with the aidl_interface module type with versions property set. The module
// type is used to build AIDL files in a way that they can be used across
// independently updatable components of the system. If a device is shipped
// with such a backward incompatible change, it has a high risk of breaking
// later when a module using the interface is updated, e.g., Mainline modules.

package vendor.amlogic.hardware.vmx_webclient;
@VintfStability
interface IVmxWebClient {
  int createInstance();
  int decrypt(in vendor.amlogic.hardware.vmx_webclient.VmxWebClientDecryptParam para, out byte[] outData);
  int decryptSecure(in vendor.amlogic.hardware.vmx_webclient.VmxWebClientDecryptParam para);
  int destroyInstance();
  void setCallback(in byte[] sessionId, in vendor.amlogic.hardware.vmx_webclient.IVmxWebClientCallback callback);
  void getProperty(in String prop, out byte[] value);
  void setProperty(in String prop, in byte[] value);
  int getCdmErr();
}
