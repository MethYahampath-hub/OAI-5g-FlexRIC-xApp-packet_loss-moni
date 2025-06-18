/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/util/alg_ds/alg/defer.h"
#include "../../../../src/util/time_now_us.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>


// xapp_gtp_mac_rlc_pdcp_moni.c

#ifndef MEAS_TYPE_PDCP_DELIVERED_SDU_COUNT
#define MEAS_TYPE_PDCP_DELIVERED_SDU_COUNT (1 << 0)
#endif

#ifndef MEAS_TYPE_PDCP_DISCARDED_SDU_COUNT
#define MEAS_TYPE_PDCP_DISCARDED_SDU_COUNT (1 << 1)
#endif


static
uint64_t cnt_mac;

static
void sm_cb_mac(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type ==INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == MAC_STATS_V0);
 
  int64_t now = time_now_us();
  if(cnt_mac % 1024 == 0)
    printf("MAC ind_msg latency = %ld μs\n", now - rd->ind.mac.msg.tstamp);
  cnt_mac++;
}

static
uint64_t cnt_rlc;

static
void sm_cb_rlc(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type ==INDICATION_MSG_AGENT_IF_ANS_V0);

  assert(rd->ind.type == RLC_STATS_V0);

  int64_t now = time_now_us();

  if(cnt_rlc % 1024 == 0)
    printf("RLC ind_msg latency = %ld μs\n", now - rd->ind.rlc.msg.tstamp);
  cnt_rlc++;
}

static
uint64_t cnt_pdcp;

// the following function has been modified extensively
// original implementation can be found in FlexRIC repository
static
void sm_cb_pdcp(sm_ag_if_rd_t const* rd, void* my_context)
{
  printf("++++ ENTERED SM_CB_PDCP ++++");
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == PDCP_STATS_V0);

  int64_t now = time_now_us();

  if(cnt_pdcp % 1024 == 0)
    printf("PDCP ind_msg latency = %ld μs\n", now - rd->ind.pdcp.msg.tstamp);

 printf("AAAAAAAAAAAA pdcp message len = %d", rd->ind.pdcp.msg.len);
 for (uint32_t i = 0; i < rd->ind.pdcp.msg.len; i++) {
    uint64_t delivered_sdu = rd->ind.pdcp.msg.rb[i].txsdu_pkts;
    // For discarded, there's no direct match, but you might consider:
    uint64_t discarded_sdu = rd->ind.pdcp.msg.rb[i].rxpdu_dd_pkts;
    printf("RB[%u]: Delivered SDUs: %" PRIu64 ", Discarded SDUs (approx): %" PRIu64 "\n", 
            i, delivered_sdu, discarded_sdu);
}
  cnt_pdcp++;
}


static
uint64_t cnt_gtp;

static
void sm_cb_gtp(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type ==INDICATION_MSG_AGENT_IF_ANS_V0);

  assert(rd->ind.type == GTP_STATS_V0);

  int64_t now = time_now_us();
  if(cnt_gtp % 1024 == 0)
    printf("GTP ind_msg latency = %ld μs\n", now - rd->ind.gtp.msg.tstamp);

  cnt_gtp++;
}


int main(int argc, char *argv[])
{
  printf("🚨===== THIS IS THE NEW BUILD ====🚨\n"); // REMOVE AFTER DEBUG

  fr_args_t args = init_fr_args(argc, argv);

  //Init the xApp
  init_xapp_api(&args);
  sleep(1);

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr_xapp(&nodes); });

  assert(nodes.len > 0);

  printf("Connected E2 nodes = %d\n", nodes.len);

  // MAC indication
  const char* i_0 = "1_ms";
  sm_ans_xapp_t* mac_handle = NULL;
  // RLC indication
  const char* i_1 = "1_ms";
  sm_ans_xapp_t* rlc_handle = NULL;
  // PDCP indication
  const char* i_2 = "1_ms";
  sm_ans_xapp_t* pdcp_handle = NULL;
  // GTP indication
  const char* i_3 = "1_ms";
  sm_ans_xapp_t* gtp_handle = NULL;


  if(nodes.len > 0){
    mac_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
    assert(mac_handle  != NULL);
    rlc_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
    assert(rlc_handle  != NULL);
    pdcp_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
    assert(pdcp_handle  != NULL);
    gtp_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
    assert(gtp_handle  != NULL);
  }

  ///////////////////////////
    // Start PERSONAL ADDITION
    ///////////////////////////

    // Generic struct def for packet data subscriptions
    typedef struct {
      uint32_t measurement_types;
      uint32_t reporting_interval_ms;
      uint32_t ue_filter;  
    } generic_report_config_t;

    // PDCP instance of struct
    generic_report_config_t pdcp_meas = {
      // .measurement_types = MEAS_TYPE_PDCP_DELIVERED_SDU_COUNT | MEAS_TYPE_PDCP_DISCARDED_SDU_COUNT,
      .reporting_interval_ms = 1000,
      .ue_filter = 0
    };

    ///////////////////////////
    // End PERSONAL ADDITION
    ///////////////////////////

  for (int i = 0; i < nodes.len; i++) {
    e2_node_connected_xapp_t* n = &nodes.n[i];
    for (size_t j = 0; j < n->len_rf; j++)
      printf("Registered node %d ran func id = %d \n ", i, n->rf[j].id);

    if(n->id.type == ngran_gNB || n->id.type == ngran_eNB){
      // MAC Control is not yet implemented in OAI RAN
      // mac_ctrl_req_data_t wr = {.hdr.dummy = 1, .msg.action = 42 };
      // sm_ans_xapp_t const a = control_sm_xapp_api(&nodes.n[i].id, 142, &wr);
      // assert(a.success == true);
      printf("==== ENTERED FIRST BRANCH ====");
      mac_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 142, (void*)i_0, sm_cb_mac);
      assert(mac_handle[i].success == true);

      rlc_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 143, (void*)i_1, sm_cb_rlc);
      assert(rlc_handle[i].success == true);

      pdcp_meas.measurement_types = MEAS_TYPE_PDCP_DELIVERED_SDU_COUNT;
      pdcp_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 144, (void*)&pdcp_meas, sm_cb_pdcp);
      assert(pdcp_handle[i].success == true);

      pdcp_meas.measurement_types = MEAS_TYPE_PDCP_DISCARDED_SDU_COUNT;
      pdcp_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 144, (void*)&pdcp_meas, sm_cb_pdcp);
      assert(pdcp_handle[i].success == true);

      gtp_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 148, (void*)i_3, sm_cb_gtp);
      assert(gtp_handle[i].success == true);

    } else if(n->id.type ==  ngran_gNB_CU || n->id.type ==  ngran_gNB_CUUP){
      printf("==== ENTERED SECOND BRANCH ====");
      pdcp_meas.measurement_types = MEAS_TYPE_PDCP_DELIVERED_SDU_COUNT;
      pdcp_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 144, (void*)&pdcp_meas, sm_cb_pdcp);
      assert(pdcp_handle[i].success == true);

      pdcp_meas.measurement_types = MEAS_TYPE_PDCP_DISCARDED_SDU_COUNT;
      pdcp_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 144, (void*)&pdcp_meas, sm_cb_pdcp);
      assert(pdcp_handle[i].success == true);

      gtp_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 148, (void*)i_3, sm_cb_gtp);
      assert(gtp_handle[i].success == true);

    } else if(n->id.type == ngran_gNB_DU){
      printf("==== ENTERED THIRD BRANCH ====");
      mac_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 142, (void*)i_0, sm_cb_mac);
      assert(mac_handle[i].success == true);

      rlc_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 143, (void*)i_1, sm_cb_rlc);
      assert(rlc_handle[i].success == true);
    }

  }

  sleep(10);


  for(int i = 0; i < nodes.len; ++i){
    // Remove the handle previously returned
    if(mac_handle[i].u.handle != 0 )
      rm_report_sm_xapp_api(mac_handle[i].u.handle);
    if(rlc_handle[i].u.handle != 0) 
      rm_report_sm_xapp_api(rlc_handle[i].u.handle);
    if(pdcp_handle[i].u.handle != 0)
      rm_report_sm_xapp_api(pdcp_handle[i].u.handle);
    if(gtp_handle[i].u.handle != 0)
      rm_report_sm_xapp_api(gtp_handle[i].u.handle);
  }

  if(nodes.len > 0){
    free(mac_handle);
    free(rlc_handle);
    free(pdcp_handle);
    free(gtp_handle);
  }

  //Stop the xApp
  while(try_stop_xapp_api() == false)
    usleep(1000);

  printf("Test xApp run SUCCESSFULLY\n");
}



