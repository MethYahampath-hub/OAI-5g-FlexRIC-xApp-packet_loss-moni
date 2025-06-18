# OAI-5g-FlexRIC-xApp-packet_loss-moni

CHANGES TO xapp_gtp_mac_rlc_pdcp_moni.c
6/18/2025: main and the pdcp callback function have been modified with the intent of subscribing to the PDCP SM and reporting both delivered and discarded SDU counts. However this xApp will fail when used with the unmodified architecture for FlexRIC.
  main has been modified as follows: definition and instances of a struct to contain data pulled from PDCP subscription call; modifications to every PDCP subscription call to report delivered/discarded SDUs instead of latency, and printf() statements to signify branches entered for debug purposes.
  pdcp callback function has been modified as follows: prinf() statements to verify that the callback function was entered during runtime (for debug purposes) and additions to report delivered and discarded SDU counts

CHANGES TO pdcp_sm_ric.c
6/18/2025: Removed assert statement from line 67 to prevent compilation failure; instead replaced it with printf("==== DEBUG:invalid input ===="); for debugging purposes. Intend to restore to original FlexRIC implementation after fixing the PDCP subscription call in xapp_gtp_mac_rlc_pdcp_moni.c
