#include <heltec.h>
#include "comm_def.h"

void Serial_comm::session() {
  if (Serial.available() > 0) {
    // serial frame detected
    slot_t* s = m_buffer->get_allocate();  // get memory allocated for reception
    byte i = 0;
    while (Serial.available() > 0) {  // get each character until empty
      char key = Serial.read();
      // detect start point
      if (key == START_SYM) {  // check for starting point
        bin = "";              // reset bin because frame is correctly detected
        // read header
        (*s).md.destination.nP = Serial.read();   // read destination net prefix
        (*s).md.destination.dID = Serial.read();  // read destination device ID
        (*s).md.origin.nP = Serial.read();        // read origin net prefix
        (*s).md.origin.dID = Serial.read();       // read origin device ID
        (*s).md.type = Serial.read();             // read type
        (*s).md.id = Serial.read();               // read message ID
        (*s).md.len = Serial.read();              // read message length

        isOnReceive = true;  // marking content reception
      }

      if (key == STOP_SYM) {     // check for frame end
        isOnReceive = false;     // mark end of frame reception
        if (i == (*s).md.len) {  // verify frame content size
          // correctly receive frame
          (*s).access_count = 1;  // mark memory slot occupied
        } else {
          // frame size mismatch
          logg += ERROR_SERIAL_FRAME_MISMATCH;
          logg += " character count=" + String(i, DEC) +
                  " specified length=" + String((*s).md.len, DEC);
        }
      }
      if (isOnReceive) {
        // content reception
        (*s).content[i++] = key;
      } else {
        // unexpected reception
        bin += key;
      }
    }
    // no character in serial reception buffer
    // reception end
    if (logg.length() > 0) {
      // events occurr!!
      reportSerialError();  // report events
    }
  }
}

void Serial_comm::reportSerialError() {
  Serial.println(logg);  // send logging message via serial
  Serial.print(" bin:");
  Serial.println(bin);   // send characters in bin slot
  logg = "";             // reset log
}
