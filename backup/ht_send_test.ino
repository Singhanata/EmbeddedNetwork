#include <heltec.h>
#include "comm_def.h"
#include "esp_task_wdt.h"

#define WDT_TIMEOUT             2
#define NET_PREFIX              'A'
#define DEVICE_ID               0
#define MEAN_SEND_DELAY         1

MSG_buffer bufferMSG;
//NET_comm netC;
MAC_comm macC;
LoRa_comm loRaC;
//Serial_comm serialC;

int msg_count = 0;

void setup() {
 Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Enable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  Serial.println("Starting LORA MODULE DEVICE***");
  Serial.println("NET PREFIX: " + (char)NET_PREFIX);
  Serial.println("DEVICE ID: " + String(DEVICE_ID, DEC));

// setup communication stack
//  netC.mac = &macC;
//  macC.net = &netC;
  macC.phy = &loRaC;
  loRaC.mac = &macC;
  loRaC.m_buffer = &bufferMSG;
//  serialC.m_buffer = &bufferMSG;

  macC.assign_device_address((byte)NET_PREFIX, (byte)('0' + DEVICE_ID));
//  netC.assign_bs_address((byte)NET_PREFIX, (byte)'0');
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
}

void loop() {
  mac_header_t h;
  meta_data_t md;
  String content = "hello";
  
  h.recipient.nP = NET_PREFIX;
  h.recipient.dID = '0';
  h.sender.nP = NET_PREFIX;
  h.sender.dID = '0'+ DEVICE_ID;

  md.destination.nP = NET_PREFIX;
  md.destination.dID = '0';
  md.origin.nP = NET_PREFIX;
  md.origin.dID = '0'+ DEVICE_ID;

  md.type = MSG_TYPE_NON;
  md.id = msg_count;
  md.len = 5;

  loRaC.onSend(h,md,content);
  msg_count++;
  Serial.println("send");
  esp_task_wdt_reset();
  int pr = MEAN_SEND_DELAY/2 + random(MEAN_SEND_DELAY) 
  delay(pr)
}
