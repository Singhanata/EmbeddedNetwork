#include <heltec.h>
#include "comm_def.h"
int lid;
int ploss;
void LoRa_comm::onReceive(int packetSize){
  //Serial.println("onreceive"); 
  if (packetSize == 0) return;          // if there's no packet, return
  slot_t temp;
  slot_t * s = &temp;
  // read packet header bytes:
  byte b = LoRa.read();
  if (b == START_SYM){
    lastRecTimeStp = millis();
    // Starting point of the packet detected
    // Begin reading header
    // save to MAC Header
    (*mac).h_rx.recipient.nP = LoRa.read();               // recipient net prefix
    (*mac).h_rx.recipient.dID = LoRa.read();              // recipient device ID
    (*mac).h_rx.sender.nP = LoRa.read();                  // sender net prefix
    (*mac).h_rx.sender.dID = LoRa.read();                 // sender device ID
    // save to the allocated memory buffer
    (*s).md.destination.nP = LoRa.read();           // destination net prefix
    (*s).md.destination.dID = LoRa.read();          // destination device ID
    (*s).md.origin.nP = LoRa.read();                // destination net prefix
    (*s).md.origin.dID = LoRa.read();               // destination device ID
    (*s).md.type = LoRa.read();                     // message type
    (*s).md.id = LoRa.read();                       // message ID
    (*s).md.len = LoRa.read();                      // message length
  } else {
    // error, start symbol is not the first character of the packet
    String f = "";
    while (LoRa.available()){
      f += (char) LoRa.read();                      // get all characters from radio
    }
    reportLoRaError(ERROR_MISSING_START_SYM, f);    // report error missing start symbol
    return;
  }
  for (int i=0; i < (*s).md.len; i++){
    (*s).content[i] = (char)LoRa.read();
  }
  // add string ending
  (*s).content[(*s).md.len] = '/0';
  b = LoRa.read();
  if (b == STOP_SYM){   
    // packet correctly received, incoke callback function
    // Serial.println(Header::tostring(*s));
    slot_t * ss = m_buffer->get_allocate();
    *ss = temp;
    mac->receive_callback(ss);                       // invoke callback function in MAC
#ifdef PHY_TEST
    performanceTest((*s).md.id);
#endif
    return;
  } else {
    String f = Header::tochar(mac->h_rx) + Header::tostring((*s));
    while (LoRa.available()){
      f += (char)LoRa.read();
    }
    s->access_count = 0;                            // set buffer slot free
    reportLoRaError(ERROR_LENGTH_MISMATCH, f);      // report error
    return;                                         // skip rest of function
  }
}

void LoRa_comm::onSend(mac_header_t h, meta_data_t md, String content){
  // build LoRa frame according to frame format
  String out = "";
  out += START_SYM;                     // start symbol 
  out += Header::tochar(h);             // recipient|sender
  out += Header::tochar(md);            // destination|origin|type|ID|length
  out += content;                       // content
  out += STOP_SYM;                      // stop symbol
  // send frame to radio module
  LoRa.beginPacket();                   // start packet
  LoRa.write(START_SYM);                // write start symbol byte
  LoRa.write(h.recipient.nP);
  LoRa.write(h.recipient.dID);
  LoRa.write(h.sender.nP);
  LoRa.write(h.sender.dID);
  LoRa.write(md.destination.nP);
  LoRa.write(md.destination.dID);
  LoRa.write(md.origin.nP);
  LoRa.write(md.origin.dID);
  LoRa.write(md.type);
  LoRa.write(md.id);
  LoRa.write(md.len);
  LoRa.print(content);                  // write LoRa frame 
  LoRa.write(STOP_SYM);                 // write stop symbol byte
  LoRa.endPacket();                     // finish and send
}

void LoRa_comm::reportLoRaError(String err, String f){
  err += " P>>" + f;                    // error message concats currupted frame characters
  Serial.println(err);                  // send message via serial port
}

void LoRa_comm::performanceTest(byte id){
  int cID = (int)id;

  // check begin 
  if (lastID == cID){
    // at the begin do nothing
  } else { 
    //not at the begin, check loop
    if (cID < lastID){
      //already loop, reset and report
      pLoss += (cID + 255) - lastID;
      Serial.print(pLoss);
      Serial.println(" packet Loss from 255 packets");
      pLoss = 0;
    } else {
      // general cases
      pLoss = cID - (lastID + 1);
    }
  }
  lastID = cID;
}

bool LoRa_comm::loRaWatchdog() {
#ifdef LORA_TIMEOUT
  if (millis() - lastRecTimeStp > LORA_TIMEOUT) {
    Serial.print(".");
    Serial.print(".");
    Serial.print(".");
    Serial.println("            ---ATTEMPT RESET LORA");
    lastRecTimeStp = millis();
    // LoRa reset
    LoRa.flush();
    LoRa.end();
    if(!LoRa.begin(BAND, true)) {
      Serial.println("ATTEMPT FALED...!");
    } else {
      Serial.println("LORA RESET... SUCCESS");
    }
  }
#endif
}