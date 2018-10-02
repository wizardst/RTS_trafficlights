// Header file for common data types
#ifndef _DATA_TYPE_H
#define _DATA_TYPE_H
#include <stdint.h>

// Message passing
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <fcntl.h>
#include <share.h>

#define X1_QNET_ATTACH_POINT "/net/s3484378-01/dev/name/local/x1_group_14" // Need to update to match node hostname

// Message types for data between nodes
typedef enum{
    MSG_TRAIN_SIGNAL = 0,
	MSG_CURRENT_STATE,
	MSG_CONTROL_STATE_LOCK,
	MSG_CONTROL_STATE_RELEASE,
	MSG_ERROR
    // Any others that need to be added here
} message_type;

// Connection errors that can be returned from function
typedef enum{
	MSG_CONNECTION_ERROR,
	MSG_SENDING_ERROR
	// Any Other error types here
} message_error;

// All nodes that will be sending/receiving data
typedef enum{
    NODE_I1 = 0,
    NODE_I2,
    NODE_X1,
    NODE_CONTROLLER
} node_id;

// This struct is what gets passed in the message
typedef struct{
	struct _pulse hdr; // This is a system header required for message passing
    node_id sending_node;
    node_id receiving_node;
    message_type msg_type; 
    uint8_t data;
} message_data_t;

#endif
