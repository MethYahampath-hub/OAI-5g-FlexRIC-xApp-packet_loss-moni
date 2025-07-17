# OAI-5g-FlexRIC-xApp-packet_loss-moni

==== CHANGES TO xapp_kpm_moni.c ====

PATH: ~/flexric/examples/xApp/c/monitor/xapp_kpm_moni.c

(07/17/2025): Discovered that there are reported KPM statistics for packet success rate and drop rate. Attempting to reconfigure example KPM xApp to subscribe to these statistics and use them to calculate packet loss.

==== CHANGES TO sm_kpm.c ==== 

PATH: ~/flexric/examples/emulator/agent/sm_kpm.c

(07/17/2025): Using the required statistics necessitates subscribing to the correct KPM data: added subscriptions to packet success and drop rate KPM statistics.

==== CHANGES TO xapp_gtp_mac_rlc_pdcp_moni.c ====

PATH: ~/flexric/examples/xApp/c/monitor/xapp_kpm_moni.c

(07/14/2025): Commented out material involving the MAC and GTP SMs because not currently necessary for packet loss calculation. REMINDER to remove entirely if xApp is made more accurate and they are still not needed. 

(07/02/2025): Packet loss statistics are now calculated per layer (PDCP and RLC). RLC packet loss will not reflect simulated changes to packet loss between UE and gNB. PDCP will reflect simulated packet loss changes, but will consistently overpredict packet loss % due to logical error. More research may be needed to fix this.

(07/01/2025): Updated the pdcp callback function to also count uplink/RX packet loss using SN.

(06/30/2025): Repaired xApp so it no longer fails with standard open architecture for FlexRIC. PDCP downlink packet statistics are now calculated using reported sequence number. Similar approach should be implemented for PDCP uplink. Unfortunately, this methodology can't be used for RLC because the SM doesn't provide sequence number reporting data.

(06/18/2025): main and the pdcp callback function have been modified with the intent of subscribing to the PDCP SM and reporting both delivered and discarded SDU counts. However this xApp will fail when used with the unmodified architecture for FlexRIC.
  main has been modified as follows: definition and instances of a struct to contain data pulled from PDCP subscription call; modifications to every PDCP subscription call to report delivered/discarded SDUs instead of latency, and printf() statements to signify branches entered for debug purposes.
  pdcp callback function has been modified as follows: prinf() statements to verify that the callback function was entered during runtime (for debug purposes) and additions to report delivered and discarded SDU counts

==== CHANGES TO pdcp_sm_ric.c ===

NO longer valid

(06/30/2025): Since xApp now works with the standard implementation for FlexRIC, DO NOT USE THIS IMPLEMENTATION FOR pdcp_sm_ric.c. Use the actual FlexRIC implementation.

(06/18/2025): Removed assert statement from line 67 to prevent compilation failure; instead replaced it with printf("==== DEBUG:invalid input ===="); for debugging purposes. Intend to restore to original FlexRIC implementation after fixing the PDCP subscription call in xapp_gtp_mac_rlc_pdcp_moni.c
