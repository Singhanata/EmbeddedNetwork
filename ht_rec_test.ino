#include <heltec.h>
#include "comm_def.h"
#include "esp_task_wdt.h"

#define WDT_TIMEOUT             2
#define NET_PREFIX              'A'
#define DEVICE_ID               0                

MSG_buffer bufferMSG;
NET_comm netC;
MAC_comm macC;
LoRa_comm loRaC;
//Serial_comm serialC;

void setup() {
 Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Enable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  Serial.println("Starting LORA MODULE DEVICE***");
  Serial.println("NET PREFIX: " + (char)NET_PREFIX);
  Serial.println("DEVICE ID: " + String(DEVICE_ID, DEC));

  // setup communication stack
  netC.mac = &macC;
  macC.net = &netC;
  macC.phy = &loRaC;
  loRaC.mac = &macC;
  loRaC.m_buffer = &bufferMSG;
//  serialC.m_buffer = &bufferMSG;

  macC.assign_device_address((byte)NET_PREFIX, (byte)('0' + DEVICE_ID));
  netC.assign_bs_address((byte)NET_PREFIX, (byte)'0');
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
}

void loop() {
  // LoRa Reception (checking new arrival packet)
  // Serial.println("loop");
  loRaC.onReceive(LoRa.parsePacket());
  // routine data process
  // node_routine();
  // serial session to send and receive data via serial port
  // serialC.session();
  esp_task_wdt_reset();
}

void node_routine(){
  slot_t * s = bufferMSG.get_FO();        // get pending message in buffer
  if (s->access_count > 0){               // check whether the slot is vacant
    Serial.print((*s).md.destination.nP);           
    Serial.print((*s).md.destination.dID);          
    Serial.print((*s).md.origin.nP);                
    Serial.print((*s).md.origin.dID);               
    Serial.print((*s).md.type);                     
    Serial.print((*s).md.id);                       
    Serial.print((*s).md.len);

  for (int i=0; i < (*s).md.len; i++){
    Serial.print((*s).content[i]);
  }
  Serial.println("");
  (*s).access_count = 0;
 }
//  netC.net_routine();
}
