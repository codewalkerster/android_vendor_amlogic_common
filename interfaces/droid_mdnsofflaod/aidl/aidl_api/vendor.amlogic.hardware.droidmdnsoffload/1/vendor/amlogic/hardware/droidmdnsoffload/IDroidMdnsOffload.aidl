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

package vendor.amlogic.hardware.droidmdnsoffload;
@VintfStability
interface IDroidMdnsOffload {
  boolean setOffloadState(boolean enabled);
  void resetAll();
  int addProtocolResponses(String networkInterface, in byte[] rawOffloadPacket, in int[] type, in int[] nameOffset);
  void removeProtocolResponses(int recordKey);
  int getAndResetHitCounter(int recordKey);
  int getAndResetMissCounter();
  boolean addToPassthroughList(String networkInterface, String qname);
  void removeFromPassthroughList(String networkInterface, String qname);
  void setWakePorts(int num, in int[] protocol, in int[] matcher, in int[] portNum);
  void setPassthroughBehavior(String networkInterface, vendor.amlogic.hardware.droidmdnsoffload.IDroidMdnsOffload.PassthroughBehavior behavior);
  parcelable MdnsProtocolData {
    byte[] rawOffloadPacket;
    List<vendor.amlogic.hardware.droidmdnsoffload.IDroidMdnsOffload.MdnsProtocolData.MatchCriteria> matchCriteriaList;
    parcelable MatchCriteria {
      int type;
      int nameOffset;
    }
  }
  enum PassthroughBehavior {
    FORWARD_ALL,
    DROP_ALL,
    PASSTHROUGH_LIST,
  }
}
