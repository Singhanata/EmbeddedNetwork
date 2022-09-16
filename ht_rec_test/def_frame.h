#ifndef _DEF_FRAME_H
#define _DEF_FRAME_H


typedef unsigned char byte;

#define DEVICE_ID_PREFIX        '0'
#define DEFAULT_GS_PREFIX       'G'
#define DEFAULT_NETWORK_PREFIX  'A'

#define BUFFER_SIZE   5
#define BUFFER_LENGTH 200

typedef volatile struct {
  unsigned int p_count;
  char bytearray[BUFFER_LENGTH];
} package_buffer_t;

// DEFINITION OF PHYSICAL CHANNEL
//#define BAND                    433E6  //you can set band here directly,e.g. 868E6,915E6

// FRAME DEFINITION ******************************************************************************
// The package is specific data frame which contained the header for the communication purpose and 
// The content for the destination endpoint. The header size is 11 bytes. It is composed of:
//      1 PACKAGE_SIZE          How many bytes are in the package including all headers and the content
//      2 RECEIVER ADDRESS      IDentity of the intended receiver in this rx/tx session.
//      3 SENDER ADDRESS        IDentity of the sender of in this rx/tx session.
//      4 DESTINATION ADDRESS   IDentity of the destination of the content inside the package
//      5 ORIGIN ADDRESS        IDentity of the originator of the content inside the package
//      6 MSG TYPE              Indicate the type of messages so that the receiver can handle the message correctly
//      7 MSG ID                The reference number of the package  
// SIZE DEFINITION
#define START_SYM_SIZE          1
#define STOP_SYM_SIZE           1
#define PACKAGE_LENGTH_SIZE     1
#define ADDRESS_SIZE            2
#define MSG_TYPE_SIZE           1
#define MSG_ID_SIZE             1
#define HEADER_SIZE             START_SYM_SIZE + PACKAGE_LENGTH_SIZE + 4*ADDRESS_SIZE + MSG_TYPE_SIZE + MSG_ID_SIZE + STOP_SYM_SIZE
#define MAX_PACKAGE_LENGTH      BUFFER_SIZE
#define MAX_CONTENT_SIZE        MAX_PACKAGE_LENGTH - HEADER_SIZE 
// POSITION DEFINITION
#define START_SYM_IDX           0
#define PACKAGE_SIZE_IDX        1
#define RECEIVER_ADDRESS_IDX    PACKAGE_SIZE_IDX + PACKAGE_LENGTH_SIZE
#define SENDER_ADDRESS_IDX      RECEIVER_ADDRESS_IDX + ADDRESS_SIZE
#define DESTINATION_ADDRESS_IDX SENDER_ADDRESS_IDX  + ADDRESS_SIZE
#define ORIGIN_ADDRESS_IDX      DESTINATION_ADDRESS_IDX  + ADDRESS_SIZE
#define MSG_TYPE_IDX            ORIGIN_ADDRESS_IDX + ADDRESS_SIZE
#define MSG_ID_IDX              MSG_TYPE_IDX + MSG_TYPE_SIZE
#define CONTENT_IDX             MSG_TYPE_IDX + MSG_ID_SIZE
// SYMBOL DEFINITION
#define START_SYM               0x28    // character '('
#define STOP_SYM                0x29    // character ')'

#define MSG_TYPE_MASK           0x30
#define MSG_TYPE_BEACON         0x00    // SIGNAL 
#define MSG_TYPE_NON            0x10    // MSG not required ACKNOWLEDGEMENT
#define MSG_TYPE_CON            0x20    // MSG required ACKNOWLEDGEMENT
#define MSG_TYPE_ACK            0x30    // ACKNOWLEDGEMENT     
// *************************************************************************************************
typedef struct {
  bool isCorrect = false;
  unsigned int size = 0;
  byte rnP = 0x00;
  byte rdID = 0x00;
  byte snP = 0x00;
  byte sdID = 0x00;
  byte dnP = 0x00;
  byte ddID = 0x00;
  byte onP = 0x00;
  byte odID = 0x00;
  byte type = 0x00;
  byte id = 0x00;
} header_t;

#endif //_DEF_FRAME_H
