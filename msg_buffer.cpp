#include <heltec.h>
#include "comm_def.h"

slot_t * MSG_buffer::get_allocate(){
  // Check available slot at the pointer index
  // pointer will go one round trip
  for (int i=0; i<BUFFER_SIZE; i++ ){
    if (slot[pointerIn].access_count > 0){
      // slot is occupied
      // increase count up to max.
      slot[pointerIn].access_count++;
      if (slot[pointerIn].access_count > MAX_ACCESS_COUNT){
        slot[pointerIn].access_count = MAX_ACCESS_COUNT;
      }
      //Increase pointer up to BUFFER SIZE
      pointerIn++;
      if (pointerIn >= BUFFER_SIZE){
        pointerIn -= BUFFER_SIZE;
      }
      continue;
    } else {
      // slot vacant return 
      return &slot[pointerIn];
    }
  }
  // pointer has rotated one round
  // free slot not found
  // overwrite the same buffer 
  buffer_overflow();
  return &slot[pointerIn];
}

slot_t * MSG_buffer::get_FO(){
  // check slot at the pointer
  for (int i=0; i<BUFFER_SIZE; i++){
    if (slot[pointerOut].access_count > 0){
      // This slot is occupied
      return &slot[pointerOut];
    }
    // the current slot is free
    pointerOut++;
    if (pointerOut >= BUFFER_SIZE){
      pointerOut -= BUFFER_SIZE;
    }
  }
  // All slot is free, return current slot
  // caution!! caller must check the slot before usage
  return &slot[pointerOut];
}

void MSG_buffer::buffer_out(byte idx){
  slot[idx].access_count = 0;
}

void MSG_buffer::buffer_overflow(){
  // TODO: send msg to keep on main processing unit before oveflow
  Serial.println("BufferException: overflow - index " + String(pointerIn, DEC) + " P>>" + Header::tochar(slot[pointerIn]));
}
