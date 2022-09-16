#include <heltec.h>
#include "comm_def.h"

void Header::copyaddress(address_t* a1, address_t* a2) {
  a1->nP = a2->nP;
  a1->dID = a2->dID;
}

void Header::setbroadcast(address_t* a1) {
  a1->nP = ADDRESS_BROADCAST;
  a1->dID = ADDRESS_BROADCAST;
}

bool Header::checkaddress(address_t a1, address_t a2) {
  return (a1.nP == a2.nP) && (a1.dID == a2.dID);
}

bool Header::checkbroadcast(address_t a1) {
  return (a1.nP == ADDRESS_BROADCAST) && (a1.dID == ADDRESS_BROADCAST);
}

String Header::tochar(address_t a) {
  String s = "";
  s += (char)a.nP;
  s += (char)a.dID;

  return s;
}

String Header::tochar(mac_header_t m) {
  String s = "";
  s += (char)m.recipient.nP;
  s += (char)m.recipient.dID;
  s += (char)m.sender.nP;
  s += (char)m.sender.dID;

  return s;
}

String Header::tochar(meta_data_t m) {
  String s = "";
  s += (char)m.destination.nP;
  s += (char)m.destination.dID;
  s += (char)m.origin.nP;
  s += (char)m.origin.dID;
  s += (char)m.type;
  s += (char)m.id;
  s += (char)m.len;

  return s;
}

String Header::tochar(slot_t sl) {
  String s = "";
  s += Header::tochar(sl.md);
  s += sl.content;

  return s;
}

String Header::tostring(mac_header_t m) {
  String s = "";
  s += "<R:";
  s += (char)m.recipient.nP;
  s += (char)m.recipient.dID;
  s += "S:";
  s += (char)m.sender.nP;
  s += (char)m.sender.dID;
  s += ">";

  return s;
}

String Header::tostring(meta_data_t m) {
  String s = "";
  s += "<D:";
  s += (char)m.destination.nP;
  s += (char)m.destination.dID;
  s += " O:";
  s += (char)m.origin.nP;
  s += (char)m.origin.dID;
  s += ">";

  if (m.type == MSG_TYPE_BEACON) {
    s += "<BEA>";
  } else if (m.type == MSG_TYPE_NON) {
    s += "<NON>";
  } else if (m.type == MSG_TYPE_CON) {
    s += "<CON>";
  } else if (m.type == MSG_TYPE_ACK) {
    s += "<ACK>";
  } else {
    s += "<UNKNOWN>";
  }
  s += "<ID:";
  s += String(m.id, DEC);
  s += "><SZ:";
  s += String(m.len, DEC);
  s += ">";

  return s;
}

String Header::tostring(slot_t sl) {
  String s = "";
  s += Header::tostring(sl.md);
  s += sl.content;

  return s;
}
