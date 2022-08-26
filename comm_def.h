#ifndef _COM_DEF_H
#define _COM_DEF_H

#define PHY_TEST
#define STATIC_NET
#define STAR_NET
  
/****************************************************************************************************
*                                        FRAME DEFINITION                                           * 
*****************************************************************************************************/
//   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7  
// |               recipient address                 |                  sender address                 |
// |              destination address                |                  origin address                 |
// |     message type       |       message ID       |    messsage length     | 

// --------------------- SIZE DEFINITION ---------------------
#define START_SYM_SIZE          1
#define STOP_SYM_SIZE           1
#define ADDRESS_SIZE            2
#define MSG_TYPE_SIZE           1
#define MSG_ID_SIZE             1
#define MSG_LENGTH_SIZE         1
#define HEADER_SIZE             START_SYM_SIZE + 4*ADDRESS_SIZE + MSG_TYPE_SIZE + MSG_ID_SIZE + MSG_LENGTH_SIZE + STOP_SYM_SIZE
#define MAX_CONTENT_SIZE        200
#define MAX_MSG_LENGTH          MAX_CONTENT_SIZE + HEADER_SIZE 
// --------------------- POSITION DEFINITION ------------------
#define START_SYM_IDX           0
#define RECEIVER_ADDRESS_IDX    START_SYM_IDX + START_SYM_SIZE
#define SENDER_ADDRESS_IDX      RECEIVER_ADDRESS_IDX + ADDRESS_SIZE
#define DESTINATION_ADDRESS_IDX SENDER_ADDRESS_IDX  + ADDRESS_SIZE
#define ORIGIN_ADDRESS_IDX      DESTINATION_ADDRESS_IDX  + ADDRESS_SIZE
#define MSG_TYPE_IDX            ORIGIN_ADDRESS_IDX + ADDRESS_SIZE
#define MSG_ID_IDX              MSG_TYPE_IDX + MSG_TYPE_SIZE
#define MSG_LENGTH_IDX          MSG_TYPE_IDX + MSG_ID_SIZE
#define CONTENT_IDX             MSG_LENGTH_IDX + MSG_LENGTH_SIZE

/**
 * PHYSICAL HEADER DEFINITION
 *  
 * Physical header defines two symbol as signal at start/stop
 */
#define START_SYM               0x13    // character '('
#define STOP_SYM                0x14    // character ')'

#define SERIAL_INVITE_SYM_1     0x11    // used for invite payload via serial port
#define SERIAL_INVITE_SYM_2     0x12    // used for invite payload via serial port

/****************************************************************************************************
*                                 MESSAGE HEADER DEFINITION                                         * 
*****************************************************************************************************/
// --------------------- ADDRESS DEFINITION ------------------
#define ADDRESS_NETWORK_PREFIX_IDX  0           // first byte is net prefix
#define ADDRESS_DEVICE_ID_IDX       1           // second byte is device ID
#define ADDRESS_BROADCAST           0xff        // always get accepted

// ------------------- MESSAGE TYPE DEFINITION ----------------
/**
 * MAC HEADER DEFINITION
 * 
 * MAC Layer defines four message types to signal the access to the channel and control tx/rx
 * 1. BEACON : protocol message - used for communication management
 * 2. NON-confirmable : low-level message - sending without attempt to gerantee a successful reception
 * 3. CONfirmable: high-level message - using retransmission and acknowledgement to confirm the outcome of communication 
 * 4. ACKnowledgement : response message - used for answering any CONfirmable message, may contain some content for 
 * communication purposes
 */
#define MSG_TYPE_MASK           0x30    // position of message type in header byte
#define MSG_TYPE_BEACON         0x00    // SIGNAL 
#define MSG_TYPE_NON            0x10    // MSG not required ACKNOWLEDGEMENT
#define MSG_TYPE_CON            0x20    // MSG required ACKNOWLEDGEMENT
#define MSG_TYPE_ACK            0x30    // ACKNOWLEDGEMENT 

#define MSG_ACK_CONTENT         "ACK"   // ACK content

// --------------------- NETWORK DEFINITION ------------------
/**
 * @brief BEACON is used for communication managements in the network layer.
 * At startup, nodes find base station and its parent. In the operation, NET.
 * will continue update PR to the base station according to outcome of transmission.
 * 
 * NET layer keeps sender as PR or BS after receiving BEACON
 */
//   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7  
// |                 parent address                  |                   child address                 |
//    0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7 
//  |      message type BEACON      |           BEACON ID           |         BEACON LENGTH         |
//  |-------|-------|-------|-------|
//  |      type     |       |  BT   |   BT - BEACON TYPE

/**
 * CON MESSAGE BS REFRESH
 * 
 * This confirmation message is periodically sent to confirm the connection with BS. As nodes receive
 * this message from their child, it will add its device id in the content and forward the message
 * to the next echelon until BS receives the message.   
 * 
 * MESSAGE FORMAT
 * 
 * HEADER
 *   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7 
 * |      message type CON         |           REQ ID           |         BEACON LENGTH         |
 * |-------|-------|-------|-------|
 * |       |   T   |           | F |   T - Message type (10)   F - Flag of BS refresh
 * 
 * CONTENT
 *   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7  
 * |                                             CON CONTENT                                           ...
 * |        BS LEVEL        |       RECORD SIZE      |                  ROUTE RECORD                   ...
 * BS LEVEL - intended hop distance from BS (Node can should to remain at a certain hop distance)
 * RECORD SIZE - route will keep increasing in size 
 * ROUTE RECORD - collection of device id
 */

/**
 * CON MESSAGE BS REFRESH RESPONSE
 * 
 * This confirmation message is used by BS to send back the confirmation after receiving CON MESSAGE BS REFRESH.
 * Upon receiving this message, nodes record its own device ID and send it towards the destination node.
 * 
 * MESSAGE FORMAT
 * 
 * HEADER
 *   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7 
 * |      message type CON         |     CORRESPONDING REQ ID      |         BEACON LENGTH         |
 * |-------|-------|-------|-------|
 * |       |   T   |       | F |   |   T - Message type (10)   F - Flag of BS response
 * 
 * CONTENT
 *   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7  
 * |                                             CON CONTENT                                           ...
 * |        BS LEVEL        |       RECORD SIZE      |                  ROUTE RECORD                   ...
 * |       RES PREFIX       |       RECORD SIZE      |                  ROUTE RECORD                   ...
 * RES PREFIX - special character (R) for signaling a roundtrip route.
 */

/**
 * BEACON PR REQUEST
 * 
 * Nodes uses this message type to register with a parent node. The receiver will consider the BS level and respond
 * to the message if it can be a parent of the sender.
 * 
 * MESSAGE FORMAT
 * 
 * HEADER
 *   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7 
 * |      message type BEACON      |             REQ ID            |         BEACON LENGTH         |
 * |-------|-------|-------|-------|
 * |       |   T   |           | F |   T - Message type (00)  F - Flag of PR request
 * 
 * CONTENT
 *   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7  
 * |                                           BEACON CONTENT                                          ...
 * |        BS LEVEL        |      No.of Children    |                List of Children                 ...
 */

/**
 * BEACON PR RESPONSE
 * 
 * Nodes uses this message type to register with a parent node. The receiver will consider the BS level and respond
 * to the message if it can be a parent of the sender.
 * 
 * MESSAGE FORMAT
 * 
 * HEADER
 *   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7 
 * |      message type BEACON      |     CORRESPONDING REQ ID      |         BEACON LENGTH         |
 * |-------|-------|-------|-------|
 * |       |   T   |       | F |   |   T - Message type (00)  F - Flag of PR response
 * 
 * CONTENT
 *   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7  
 * |                                           BEACON CONTENT                                          ...
 * |          QUOTA         |       RECORD SIZE      |                  ROUTE RECORD                   ...
 * QUOTA - update the quota of registered children from parent
 */

/**
 * BEACON PR CONFIRM
 * 
 * Nodes uses this message type to register with a parent node. The receiver will consider the BS level and respond
 * to the message if it can be a parent of the sender.
 * 
 * MESSAGE FORMAT
 * 
 * HEADER
 *   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7 
 * |      message type BEACON      |     CORRESPONDING REQ ID      |         BEACON LENGTH         |
 * |-------|-------|-------|-------|
 * |       |   T   |   | F |       |   T - Message type (00)  F - Flag of PR CONFIRM
 * 
 * CONTENT
 *   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7   0  1  2  3  4  5  6  7  
 * |                                        NO CONTENT FORMAT                                          ...
 */
// BEACON PREFIX shows the progress of conversation
#define BEACON_RES_BIT          0x01    // bit check for "response" beacon  
#define BEACON_PR_BIT           0x10    // bit check for "parent" beacon

#define CON_FLAG_BS_REFRESH     0x01    // CON message to BS for route report
#define CON_FLAG_BS_RES         0x02    // CON message from BS to confirm report
#define BEACON_FLAG_PR_REQ      0x01    // beacon for parent request
#define BEACON_FLAG_PR_RES      0x02    // beacon for response of PR REQ
#define BEACON_FLAG_PR_CON      0x04    // beacon for confirmation of RES

#define CON_PREFIX_BS_RES       'R'     // signal the end of the forward route begin the backward route

/****************************************************************************************************
*                                     PROTOCOL DEFINITION                                           * 
*****************************************************************************************************/
// -------------------- physical tx/rx ----------------
// CHANNEL DEFINITION
#define BAND    433E6
// ERROR OF DATA FRAME RECEPTION
#define ERROR_MISSING_START_SYM     "LoRaReceptionError: incorrect format - first symbol must be START_SYM"
#define ERROR_LENGTH_MISMATCH       "LoRaReceptionError: incorrect format - content length mismatch"
// --------------------- MAC SESSION ------------------
/**
 * Transmission of confirmable message uses a session to save the state of communication
 * After the first transmission, the message will be periodically sent until the corresponding ACK 
 * is accepted or the attempt quota has been reached. 
 */
#define CON_SESSION_ATTEMPT_INTERVAL  3000        // interval between attempt
#define CON_SESSION_MAX_ATTEMPT       3           // attempt quota
// -------------------- NET PROTOCOL ------------------
#define MAX_NODE_RECORD               20          // max. number of node records
#define MAX_HOP                       3           // max. hop distance
#define ROUTE_SIZE                MAX_HOP - 1     // size of route to BS

#define NET_STAT_BS_BIT               0x01        // flag of node being BS
#define NET_STAT_BS_KNOWN             0x02        // flag showing that node is already registered
#define NET_STAT_PR_REQ               0x04        // flag showing that request is on process
#define NET_STAT_PR_ACTIVE            0x08        // flag showing that node already registered by a parent node
#define NET_STAT_BS_CONFIRM           0x10        // flag showing that a confirmation from BS is received
#define NET_STAT_PR_UPDATE            0x20        // flag showing that PR must be updated

#define NET_UPDATE_INTERVAL           60000       // update period of NET layer
#define NET_REQ_INTERVAL              10000       // retry a request
#define ROUTE_QUALITY_MAX             3           // max. route quality
#define ROUTE_QUALITY_THRESHOLD       -3          // threshold to attempt new route
#define EXPECT_CHILD_COUNT            4           // optimum child node count

#define REQ_OUTCOME_COMPLETE          0           // request is correctly handled
#define REQ_ERR_OUT_OF_DOMAIN         1           // request not from this net domain
#define REQ_ERR_QUOTA_OUT             2           // request not received due to lack of quota
#define REQ_ERR_BS_LVL_NA             3           // request not received due to low BS level

#define RES_OUTCOME_COMPLETE          0           // response is correctly handled    
#define RES_ERR_NOT_FOR_ME            1           // response error (client address mismatch)
#define RES_ERR_ID_MISMATCH           2           // response error ID mismatch
#define RES_ERR_SIZE_MISMATCH         3           // response error content size mismatch

#define ERROR_COMM_TX_FAIL           "TransmissionFailed: ATTEMPT QUOTA OUT - "
// -------------------- SERIAL SESSION ----------------
#define ERROR_SERIAL_FRAME_MISMATCH  "SerialReceptionError: incorrect format - frame length mismatch"
// ----------------------- BUFFER ---------------------
#define BUFFER_SIZE                   5           // number of available slots
#define MAX_ACCESS_COUNT              250         // maximum value of attempt count (TODO: count attemt on access)

/****************************************************************************************************
*                                       TYPE DEFINITION                                            * 
*****************************************************************************************************/
// address
typedef struct {
  byte nP = 0;                        // net prefix
  byte dID = 0;                       // device ID
} address_t;
// MAC protocol header 
typedef struct {
  address_t recipient;                // recipient address
  address_t sender;                   // sender address
} mac_header_t;
// higher communication protocol header
typedef struct {
  address_t destination;              // destination address
  address_t origin;                   // origin address   
  byte type = 0;                      // message type
  byte id = 0;                        // message ID
  byte len = 0;                       // message length
} meta_data_t;
// memory slot allocated for storing a message
typedef struct {
  byte access_count = 0;              // positive value shows the slot is occupied
  meta_data_t md;                     // meta data of message
  char content[MAX_CONTENT_SIZE];     // content of message in char array
} slot_t;
// state of the CON session
typedef struct {
  bool isOnProcess;                   // flag for ongoing CON session
  unsigned long last_attempt;         // time stamp of the last transmission
  byte attempt_count;                 // transmission count
  mac_header_t h;                     // mac header 
  byte slot_idx;                      // slot index of the CON message
  slot_t * slot;                      // pointer to CON message
} con_session_t;
// route record
typedef struct {
  byte hop[MAX_HOP];                  // device id of the intermidiate nodes
} route_t;
// neighbour record
typedef struct {
  bool isActive = false;              // is node active?
  byte id;                            // device id
  byte nexthop;                       // device id of nexthop           
} subnode_t;
// network status information
typedef struct {
  byte status = 0;                    // net status
  long lastUpdate = 0;                // time stamp of last update
  signed char route_eval = 0;         // evaluate the success rate of communication
  byte child_count = 0;               // number of children registered with node
  byte child_quota = 3;               // quota of children
  byte bs_level = 0xFF;               // hop distance from BS
  route_t selfRoute;                  // route of this node
} net_status_t;
// network session
typedef struct {
  bool isActive = false;              // is session active
  byte id = 0;                        // req id
  long lastAttempt = 0;               // time stamp of last attempt
  slot_t beacon_holder;               // message holder
} req_session_t;

/****************************************************************************************************
*                                       CLASS DEFINITION                                            * 
*****************************************************************************************************/
// ------------------------ DECLARATION -------------------- 
/**
 * Utility class for frame headers
 * 
 * Structs of headers is translated to readable String object 
 * or char arrays for transmission
 */
class Header;
/**
 * @brief LoRa_comm defined the data frame tx/rx
 * according to the defined format 
 * 
 * The class is built on top of the library "Heltec ESP32 Dev-Boards"
 * The data frame is sent in the String format (char array)
 */
class LoRa_comm;
/**
 * @brief MAC_comm defines the behaviour of MAC protocol. 
 * This network uses simple ALOHA.  
 *   
 * The packet loss over comm link can be avoided by acknowledgement.
 */
class MAC_comm;
/**
 * @brief The network protocol decides the route to destination 
 * upon the reception of BEACON
 * 
 * This class handle the reception of BEACON and message.
 * 1. NON - If not for this device, send it to nexthop immediately
 * 2. CON - save it on buffer
 * 3. BEACON  - As Endnode, get DECLARE, save the origin as PR or BS. send DECLARE back to confirm
 *              As parent, get DECLARE, save the sender as child node 
 *            - get REQ, if PR is known or the node itself is FW
 * 4. ACK - record the successful transmission
 */
class NET_comm;
/**
 * FIFO message buffer, implemented by rotating pointer
 * 
 * The buffer slot is structured according to the message format.
 * The pointer indicates the slot index for storing and accessing.
 * The pointer increments in one direction as the slot is occupied.  
 */
class MSG_buffer;

// --------------------- HEADER CLASS -----------------
class Header {
  public:
    /// translate address to char array 
    static String tochar(address_t a);
    /// translate mac_header_t to char array
    static String tochar(mac_header_t m);
    /// translate meta_data_t to char array
    static String tochar(meta_data_t m);
    /// translate slot_t to char array
    static String tochar(slot_t sl);

    /// translate mac_header_t to readable String
    static String tostring(mac_header_t m);
    /// translate meta_data_t to readable String
    static String tostring(meta_data_t m);
    /// translate slot_t to readable String
    static String tostring(slot_t sl);

    /// is address a1 equals to address a2
    static bool checkaddress(address_t a1, address_t a2);
    /// is address broadcast 
    static bool checkbroadcast(address_t a1);
    /// copy origin to copy
    static void copyaddress(address_t * copy, address_t * origin);
    /// assign broadcast to a1
    static void setbroadcast(address_t * a1);
};

class MSG_buffer {
  private:
    byte pointerIn = 0;
    byte pointerOut = 0;
    slot_t slot[BUFFER_SIZE];
    /// protocol of the buffer overflow event 
    void buffer_overflow();

  public:
    /// get the pointer of the free slot 
    slot_t * get_allocate();
    /// get the pointer of the oldest slot
    slot_t * get_FO();
    /// marking the slot as free
    void buffer_out(byte idx);
};

class NET_comm {
  private:
    net_status_t net_stat; 
    req_session_t pr_session;
    req_session_t bs_session;

    address_t bs_address;
    address_t pr_address;
    
    subnode_t subnode[MAX_NODE_RECORD];

    /**
     * Find next hop for a given destination
     * 
     * @param dID destination device ID
     * @return ** byte nexthop device ID
     */
    byte find_nexthop(byte dID);
    /// request a refresh of network status
    void net_request(long now);
    /// set pr session
    void set_pr_session(byte flag);
    /// send BEACON RES
    void send_response(slot_t * s);
    /// send message via serial port
    void serial_send(slot_t * s);
    /// build beacon message according to format
    void beacon_create(slot_t * s,
          address_t * destination,
               address_t * origin,
                        byte type, 
                          byte id);
    /**
     * handle REQ 
     * 
     * @param s pointer to allcated memory slot containing beacon
     * @return ** byte status of outcome 
     */
    byte read_request(slot_t * s); 
    /**
     * handle RES
     * 
     * @param s pointer to allcated memory slot containing beacon
     * @return ** byte status of outcome 
     */
    byte read_response(slot_t * s);
    /**
     * record subnode in list
     * 
     * @param dID device ID of subnode
     * @param nID device ID of nexthop
     * @return true requested subnode is in the record list
     * @return false requested subnode is not in the record list
     */
    bool register_subnode(byte dID, byte nID);

  public:

    MAC_comm * mac;
    /// assign base station address
    void assign_bs_address(byte nP, byte dID);
    /// assign parent address
    void assign_pr_address(byte nP, byte dID);

    /**
     * @brief this function is called after the BEACON reception process in
     * MAC layer is finished to process the message according to
     * the protocol in NET layer.
     * 
     * BEACON is used for handcheck process to determine BS and PR
     * 
     * @param h MAC header {recipient address sender address}  
     * @param s pointer to allcated memory slot containing message
     */
    void beacon_callback(mac_header_t * h, slot_t * s);
    /**
     * @brief this function is called after the CON reception process in
     * MAC layer is finished to process the message according to
     * the protocol in NET layer.
     * 
     * NON message will be immediately forwarded, if it is not for this node 
     * 
     * @param s pointer to allcated memory slot containing message
     */
    void non_callback(slot_t * s);
    /**
     * @brief this function is called after the CON reception process in
     * MAC layer is finished to process the message according to
     * the protocol in NET layer.
     * 
     * @param h MAC header {recipient address sender address}
     * @param s pointer to allcated memory slot containing message
     */
    void con_callback(slot_t * s);
    /**
     * @brief this function is called after the ACK reception process in
     * MAC layer is finished to process the message according to
     * the protocol in NET layer.
     * 
     * @param h MAC header {recipient address sender address}
     */    
    void ack_callback(mac_header_t * h);
    /**
     * @brief this function is called in cases of unsuccessful transmission
     * according to the protocol in NET layer.
     * 
     * @param h MAC header {recipient address sender address}
     * @param s pointer to allcated memory slot containing message
     */    
    void fail_callback(mac_header_t * h, slot_t * s);
    /**
     * @brief this function is called in cases of overhearing packet for another node
     * according to the protocol in NET layer.
     * 
     * @param h MAC header {recipient address sender address}
     */    
    void overhear_callback(mac_header_t * h);
    /**
     * @brief routine process of NET layer. Update the parameters
     * in a defined period
     * 
     * @return ** void 
     */
    void net_routine();
};

class MAC_comm {
  private:
    con_session_t session;
    /**
     * @brief Send ACK after reception of CON message
     * 
     * @param s pointer to allcated memory slot containing message
     * @return ** void 
     */
    void mac_send_ack(slot_t * s);
    ///
    void set_con_session(slot_t * s);

  public:
    NET_comm * net;
    LoRa_comm * phy;
    mac_header_t h_rx;
    address_t device_address;
    /// check time, continue CON MESSAGE session 
    void mac_routine();
    /// record device address
    void assign_device_address(byte nP, byte dID);
    /**
     * @brief function is called after the correct reception of
     * physical data frame 
     * 
     * @param s pointer to allcated memory slot containing message
     * @return true frame is accepted by MAC layer (address not matched)
     * @return false frame is not accepted by MAC layer
     */
    bool receive_callback(slot_t * s);
    /**
     * @brief MAC defines two method for sending messages:
     * 1. non-confirmable - "send and forget" principle
     * 2. confirmable - build a session for retransmission and ACK
     * 
     * @param h MAC header {recipient address sender address}
     * @param s pointer to allcated memory slot containing message
     * @return is CON session available
     */
    bool onSend(mac_header_t h, slot_t * s);
};

class LoRa_comm {
  private:
    byte lastID = 0;
    int pLoss = 0;
    /// report error in the reception process
    void reportLoRaError(String err, String f);
    void performanceTest(byte id);
    void lora_clear(String err);
  public:
    MSG_buffer * m_buffer;
    MAC_comm * mac;
    /**
     * @brief This callback function is called after parsing LoRa packet
     * 
     * The reception process detects start/stop of the data frame and record
     * the received bytes according to data frame format.
     * 
     * @param packetSize size of char array received by LoRa radio 
     * @return ** void 
     */
    void onReceive(int packetSize);
    /**
     * @brief build data frame according to the defined format
     * and send on loRa radio 
     * 
     * @param h MAC header {recipient|sender}
     * @param md meta data of message {destination|origin|type|ID|size}
     * @param content content of message
     * @return ** void 
     */
    void onSend(mac_header_t h, meta_data_t md, String content);
};

class Serial_comm {
  private:
    String bin = "";
    String logg = "";
    bool isOnReceive = false;
    /// report error in the serial reception process
    void reportSerialError();

  public:
    MSG_buffer * m_buffer;
    /**
     * @brief The serial reception in cases of pending characters 
     * in serial rx buffer
     * 
     * @return ** void 
     */
    void session();
};

class Process_routing {
  public:
};
#endif //_COM_DEF_H