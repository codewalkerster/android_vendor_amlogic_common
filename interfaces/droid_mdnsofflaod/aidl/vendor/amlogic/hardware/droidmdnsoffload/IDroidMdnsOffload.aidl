package vendor.amlogic.hardware.droidmdnsoffload;

@VintfStability
interface IDroidMdnsOffload {
     parcelable MdnsProtocolData {
         parcelable MatchCriteria {
             int type;
             int nameOffset;
         }

         byte[] rawOffloadPacket;
         List<MatchCriteria> matchCriteriaList;
     }

     boolean setOffloadState(boolean enabled);
     void resetAll();
     int addProtocolResponses(String networkInterface, in byte[] rawOffloadPacket, in int[] type, in int[]nameOffset);
     void removeProtocolResponses(int recordKey);
     int getAndResetHitCounter(int recordKey);
     int getAndResetMissCounter();
     boolean addToPassthroughList(String networkInterface, String qname);
     void removeFromPassthroughList(String networkInterface, String qname);

     enum PassthroughBehavior {
          FORWARD_ALL,
          DROP_ALL,
          PASSTHROUGH_LIST,
     }
     void setPassthroughBehavior(String networkInterface, PassthroughBehavior behavior);
}
