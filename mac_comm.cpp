#include <heltec.h>
#include "comm_def.h"

void MAC_comm::assign_device_address(byte nP, byte dID){
  device_address.nP = nP;                                     // set net prefix
  device_address.dID = dID;                                   // set device address
}

void MAC_comm::mac_routine(){
  long now = millis();
  if (now - session.last_attempt > CON_SESSION_ATTEMPT_INTERVAL){
    // time for re-attempt
    if (session.attempt_count < CON_SESSION_MAX_ATTEMPT){     // still on quota
      phy->onSend(session.h, (*session.slot).md, (*session.slot).content);
      session.attempt_count++;                                // attempt count
    } else { // out of quota 
      // clear session and report
      session.isOnProcess = false;                            // stop session
      session.attempt_count = 0;                              // clear session
      net->fail_callback(&session.h, session.slot);           // fail report
    }
  } 
}

bool MAC_comm::receive_callback(slot_t * s){
  bool isToThis = Header::checkaddress(device_address, h_rx.recipient); // check device address 
  bool isReceiver = (isToThis||Header::checkbroadcast(h_rx.recipient)); // check broadcast address & device address
  if(!isReceiver) { //Overhearing Packets not meant for this node           
          net->overhear_callback(&h_rx);               
  }
  byte type = ((*s).md.type & MSG_TYPE_MASK);
  switch (type){
  case MSG_TYPE_BEACON:
    // check address 
    // invoke network callback
    (*s).access_count = 1;                          // buffer slot marked
    net->beacon_callback(&h_rx, s);                 // callback NET layer
    return true;
  case MSG_TYPE_NON:
    if (isReceiver) {
      // message correctly received
      (*s).access_count = 1;                        // buffer slot marked
      net->non_callback(s);                         // callback NET layer      
    }
    return true;
  case MSG_TYPE_CON:
    if (!isToThis) {return false;}
    (*s).access_count = 1;                          // buffer slot marked
    net->con_callback(s);                           // callback NET layer
    mac_send_ack(s);                                // send ACK to sender
    return true;
  case MSG_TYPE_ACK:
    if (!isToThis) {return false;}
    if ((*session.slot).md.id == (*s).md.id){       // check ID
      // clear session
      (*s).access_count = 1;                        // buffer slot marked
      session.isOnProcess = false;                  // close the CON session
      session.attempt_count = 0;                    // reset attempt count
      net->ack_callback(&h_rx);                     // report outcome    
    }
    return true;
  }
}

bool MAC_comm::onSend(mac_header_t h, slot_t * s){
  if ((*s).md.type & MSG_TYPE_CON == (*s).md.type & MSG_TYPE_CON){    // CON message
    // session must be established before sending
    session.isOnProcess = true;                     // set ongoing CON session
    session.last_attempt = millis();                // set time stamp
    session.h = h;                                  // FIXME: bug suspected, set header 
    session.slot = s;                               // set pointer to message
  } 
  phy->onSend(h, (*s).md, (*s).content);            // send to physical layer
}

void MAC_comm::mac_send_ack(slot_t * s){
  mac_header_t h_ack;                                     // MAC header for ACK
  meta_data_t md_ack;                                     // meta data of ACK
  String content = MSG_ACK_CONTENT;                       // set ACK content
  
  Header::copyaddress(&h_ack.recipient, &h_rx.sender);    // set recipient as sender of message
  Header::copyaddress(&h_ack.sender, &device_address);    // set sender as device address

  md_ack = (*s).md;                                       // set meta data
  md_ack.len = content.length();                          // set content length

  phy->onSend(h_ack, md_ack, content);                    // send packet
}
