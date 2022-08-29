#include <heltec.h>
#include "comm_def.h"

// FIXME: REQ may result in getting the same bad PR 
void NET_comm::beacon_callback(mac_header_t * h, slot_t * s){
  // check BEACON type
  byte type = (*s).md.type;
  // separate type by examining flag                                   
  if (type & BEACON_FLAG_PR_REQ){                             // BEACON REQ 
    byte outcome = read_request(s);                           // process flow of BEACON REQ
    if (!outcome){                                            // successful outcome
      net_stat.status |= NET_STAT_PR_UPDATE;                  // mark PR need to be updated 
      NET_comm::send_response(s);                                       // send response
    } else {
      // TODO: report error: request invalid
    }
    s->access_count = 0;                                      // mark slot as done
  }
  if (type & BEACON_FLAG_PR_RES){
    byte outcome = read_response(s);
    if (!outcome){                                            // receive RES correctly
      net_stat.status &= ~NET_STAT_PR_REQ;                    // mark end of PR REQ
      net_stat.status |= NET_STAT_PR_ACTIVE;                  // mark PR active
      
      net_stat.status |= NET_STAT_BS_KNOWN;                   // mark BS known
      net_stat.status &= ~NET_STAT_BS_CONFIRM;                // mark BS unconfirmed
    } else {
      // TODO: report error: response invalid
    }                               
  }
}

void NET_comm::non_callback(slot_t * s){
  // check destination
  if (Header::checkaddress((*s).md.destination, mac->device_address)){  // check destination address
    // message to this node
    (*s).access_count = 0;    // mark memory slot occupied
    serial_send(s);           // send packet via serial port
    performanceTest(&(*s).md.origin, (*s).md.id);
  } else {                    // not for this node
    // forward immediately
    if ((*s).md.destination.nP == (*mac).device_address.nP){  // check net prefix
      byte dID = (*s).md.destination.dID;                     // check destination device ID
      byte nID = NET_comm::find_nexthop(dID);                 // search net entry get nexthop device ID
      mac_header_t h;
      h.recipient.nP = (*mac).device_address.nP;              // set net prefix of recipient
      h.recipient.dID = nID;                                  // set device ID of recipient
      Header::copyaddress(&h.sender, &mac->device_address);   // set sender
      mac->onSend(h, s);                                      // send packet
    } /*not belong to the same domain */
  }
}

void NET_comm::con_callback(slot_t * s){
  if(Header::checkaddress((*s).md.destination,mac->device_address)){  // for this node 
    if ((*s).md.type & CON_FLAG_BS_REFRESH){
      
    }
  }  
  (*s).access_count = 1;                                          // mark memory slot occupied
}

void NET_comm::ack_callback(mac_header_t * h){
  if (Header::checkaddress(h->sender, pr_address)){               // check PR address
    net_stat.route_eval++;                                        // plus route evaluation
    if (net_stat.route_eval > ROUTE_QUALITY_MAX){                 // evaluation too high
      net_stat.route_eval = ROUTE_QUALITY_MAX;                    // set to max point
    }
  }
}

byte NET_comm::find_nexthop(byte dID){
#ifndef STATIC_NET
  for (int i=0; i<net_stat.child_count; i++){            
    if (subnode[i].isActive){                                     // device is active
      if (subnode[i].id == dID){                                  // destination is descendant
        return subnode[i].nexthop;                                // return nexthop value
      } 
    }
  }
  return pr_address.dID;                                          // destination not belong to this node
#else
#ifdef STAR_NET
  return 0;
#else
  if (dID > 0) {
    return (dID-1);
  } else {
    return 0;
  }
#endif
#endif
}

void NET_comm::fail_callback(mac_header_t * h, slot_t * s){
  String err = ERROR_COMM_TX_FAIL;                                // set error message
  String hS = Header::tostring(*h);                               // concat header
  String sS = Header::tostring(*s);                               // concat message content
  Serial.println(err + "P>>" + hS + sS);                          // report outcome via serial port
  s->access_count = 0;                                            // unmark the memory slot

  net_stat.route_eval--;                                          // minus evaluation point 
  if (net_stat.route_eval < ROUTE_QUALITY_THRESHOLD){             // evaluation below threshold
    net_stat.route_eval=ROUTE_QUALITY_THRESHOLD;                  // set to min. route evaluation
  }
}

void NET_comm::overhear_callback(mac_header_t * h) {
  // TODO 
}

void NET_comm::net_routine(){
  long now = millis();
  if (now - net_stat.lastUpdate > NET_UPDATE_INTERVAL){           // check update time
    // update time has been reached
    net_stat.status &= ~NET_STAT_BS_CONFIRM;                      // get new confirmation from BS
    net_stat.lastUpdate = now;                                    // set time stamp of last update
  }
  NET_comm::net_request(now);                                     // send message for network status update
}

void NET_comm::net_request(long now){
  if (!(net_stat.status & NET_STAT_PR_ACTIVE)){                   // no PR registered
    // node has no parent
    if (!(net_stat.status & NET_STAT_PR_REQ)){                    // still not requesting
      // node still not on request session
      NET_comm::set_pr_session(NET_STAT_PR_REQ);                  // set PR requesting
    } 
  } else {
    // PR is already active
    if (net_stat.status & NET_STAT_PR_UPDATE) {                   // PR must be updated
      // new information for PR
      NET_comm::set_pr_session(NET_STAT_PR_UPDATE);               // set PR requesting
    }
  }
}

void NET_comm::set_pr_session(byte flag){
  if (pr_session.isActive){return;}                               // If PR session is on going, wait for finish
  mac_header_t h;                                                 // sender and recipient
  pr_session.isActive = true;                                     // set session active
  pr_session.lastAttempt = millis();                              // set time stamp 
  // build BEACON PR REQ
  if (flag & NET_STAT_PR_REQ){
    // request new PR
    address_t broadcast;
    Header::setbroadcast(&broadcast);
    Header::setbroadcast(&h.recipient);                           // no specific recipient
    Header::copyaddress(&h.recipient, &mac->device_address);      // sender
    NET_comm::beacon_create(&pr_session.beacon_holder,            // BEACON PR REQ records on slot 
                                         &broadcast,              // no parent address 
                                 &mac->device_address,            // client address
                                   BEACON_FLAG_PR_REQ,            // beacon type
                                       pr_session.id);            // request ID
  }
  if (flag & NET_STAT_PR_UPDATE){
    // update information by report
    Header::copyaddress(&h.sender, &pr_address);                  // no specific recipient
    Header::copyaddress(&h.recipient, &mac->device_address);      // sender
    NET_comm::beacon_create(&pr_session.beacon_holder,            // BEACON PR REQ records on slot 
                                          &pr_address,            // parent address 
                                 &mac->device_address,            // client address
                                   BEACON_FLAG_PR_REQ,            // beacon type
                                       pr_session.id);            // request ID

  }
  byte len = 0;                                                             // content length
  pr_session.beacon_holder.content[len++] = ('0' + net_stat.bs_level);      // content 1 bs level
  pr_session.beacon_holder.content[len++] = ('0' + net_stat.child_count);   // content 2 length of children
  for (int i=0; i<net_stat.child_count; i++){
    pr_session.beacon_holder.content[len++] = subnode->id;                  // record entry of child ID
  }
  pr_session.beacon_holder.md.len = len;                                    // set message length

  if(mac->onSend(h , &pr_session.beacon_holder)) {                // send successfully
    pr_session.beacon_holder.access_count = 1;                    // mark occupied
  }
}

void NET_comm::send_response(slot_t * s){
  mac_header_t h;
  // set mac header
  Header::copyaddress(&h.recipient, &(*s).md.origin);             // client address
  Header::copyaddress(&h.sender, &(*mac).device_address);         // this device address
  // set meta data
  NET_comm::beacon_create(&pr_session.beacon_holder,
                               &mac->device_address,              // set this node as parent
                                    &(*s).md.origin,              // set client address
                                 BEACON_FLAG_PR_RES,              // set type as BEACON RES
                                         (*s).md.id);             // corresponding id

  byte len = 0;
  /**
   * Quota calculation
   * 
   * Node can have a limit number of subnodes so that the total number of nodes 
   * in the network does not exceed the capacity of the net work.
   * This is the beta version of quota distribution using a primitive principle.
   * Each node is estimated to have a certain number of direct children. 
   * Each child is given by equal amount of quota, therefore the given quota
   * is calculate by: (totalQuota/numOfChildren - 1)
   * For example, given quota of 20 and 4 children -> each child is given 20/4 - 1 = 4  
   */
  byte quota = net_stat.child_quota/EXPECT_CHILD_COUNT;
  if (quota > 0){quota--;}
  pr_session.beacon_holder.content[len++] = quota;
  // route record
  pr_session.beacon_holder.content[len++] = net_stat.bs_level;    // extract meaningful hop  
  for (int i = 0; i < net_stat.bs_level; i++){        
    pr_session.beacon_holder.content[len++] = net_stat.selfRoute.hop[i];  // record each hop
  }
  (*s).md.len = len;
  if (mac->onSend(h, &pr_session.beacon_holder)){                 // successfully sent
    pr_session.beacon_holder.access_count = 0;                    // TODO: mark beacon holder vacant
  }
}

byte NET_comm::read_request(slot_t * s){
  // separate update from new request
  if (((*s).md.origin.nP != (*mac).device_address.nP) ||
          ((*s).md.origin.nP != bs_address.nP)){return REQ_ERR_OUT_OF_DOMAIN;}  // check net domain
  byte bs_level = s->content[0] - '0';                            // extract hop distance from BS
  byte child_count = s->content[1] - '0';                         // extract child count
  if (bs_level > net_stat.bs_level){                              // this node is nearer to BS
    if (net_stat.child_quota > (child_count + 1)){                // this node still have enough quota 
      register_subnode((*s).md.origin.dID, (*s).md.origin.dID);   // record sender as subnode 
      for (int i = 0; i < child_count; i++){
        register_subnode((*s).content[1+i], (*s).md.origin.dID);  // record all subnodes of sender as subnode
      }
      return REQ_OUTCOME_COMPLETE;
    } else {
      return REQ_ERR_QUOTA_OUT;
    }
  } else {
    return REQ_ERR_BS_LVL_NA;
  }
}

byte NET_comm::read_response(slot_t * s){
  // PR sent RES back
  // check corresponding RES
  if (!Header::checkaddress((*s).md.origin, mac->device_address)){return RES_ERR_NOT_FOR_ME;}
  if ((*s).md.id != pr_session.id){return RES_ERR_ID_MISMATCH;}
  // message is really RES from this node 
  pr_address = (*s).md.destination;                         // record parent address
  byte len = 0;                                           
  byte quota = (*s).content[len++];                         // record quota
  byte sz = (*s).content[len++];                            // route size
  for (int i = 0; i < sz; i++){
    net_stat.selfRoute.hop[i] = (*s).content[len++];        // route hop record
  }
  net_stat.selfRoute.hop[sz] = pr_address.dID;              // add parent node to route
  net_stat.bs_level = (sz+1);                               // record BS hop distance
  if ((*s).md.len != len){return RES_ERR_SIZE_MISMATCH;}    // check content size
  
  quota -= (net_stat.child_count + net_stat.child_quota);   // calculate remaining quota
  if (quota < 0){                                           // check negative quota 
    quota = 0;                                              // set to zero
  }
  net_stat.child_quota = quota;                             // set remaining quota
  /*end of process*/
}

void NET_comm::beacon_create(slot_t * s, address_t * destination, 
                                   address_t * origin, byte type, byte id){
  // write address 
  Header::copyaddress(&(*s).md.destination, destination);         // write intended receiver address
  Header::copyaddress(&(*s).md.origin, origin);                   // write sender of network
  s->access_count = 1;                                            // flag the invacant state
  // write meta data
  (*s).md.type = MSG_TYPE_BEACON;                                 // write beacon type
  (*s).md.type |= type;                                           // flag the beacon type
  (*s).md.id = id;                                                // message ID
}

bool NET_comm::register_subnode(byte dID, byte nID){
  if (net_stat.child_quota > 0){                      // quota is not out
    for (int i=0; i<net_stat.child_count; i++){
      if (subnode[i].id == dID){                      // check id in database
        subnode[i].isActive = true;                   // set active
        return true;                                  // return correctly record
      }
    }
    subnode[net_stat.child_count].id = dID;           // record device address
    subnode[net_stat.child_count].nexthop = nID;      // next hop device address
    subnode[net_stat.child_count].isActive = true;    // set active
    net_stat.child_count++;                           // add child count
    net_stat.child_quota--;                           // reduce quota;
    return true;                                      // return correctly record
  } 
  return false;                                       // return out of quota
}

void NET_comm::assign_bs_address(byte nP, byte dID){
  bs_address.nP = nP;
  bs_address.dID = dID;
}

void NET_comm::assign_pr_address(byte nP, byte dID){
  pr_address.nP = nP;
  pr_address.dID = dID;
}

void NET_comm::serial_send(slot_t * s){
  String out = Header::tostring(*s);
  Serial.println(out);
}

void NET_comm::performanceTest(address_t * s, byte id){
  // Serial.println(" NET Eval");
  int cID = (int)id;
  // Serial.print("cID");
  // Serial.println(cID);
  int idx = (*s).dID - 48;
  // Serial.print("idx");
  // Serial.println(idx);
  int lastID = (int)subnode[idx].mID;
  // Serial.print("lastID");
  // Serial.println(lastID);
  // check begin 
  if (lastID == cID){
    // at the begin do nothing
  } else { 
    //not at the begin, check loop
    if (cID < lastID){
      //already loop, reset and report
      subnode[idx].pLoss += (cID + 255) - lastID;
      Serial.print("N");
      Serial.print(idx);
      Serial.print(": ");
      Serial.print(subnode[idx].pLoss);
      Serial.println(" packet Loss from 255 packets");
      subnode[idx].pLoss = 0;
    } else {
      // general cases
      subnode[idx].pLoss = cID - (lastID + 1);
    }
  }
  subnode[idx].mID = cID;
}