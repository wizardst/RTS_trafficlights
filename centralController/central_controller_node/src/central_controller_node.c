#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <global_include.h>
#include <x1_include.h>
#include <controller_include.h>

#define LOCAL_ATTACH_POINT "c1_group_14"

/*Function Declarations*/
void* c1_message_server_thread(void*);
void c1_global_init(controller_data_t*);
void* print_x1_state(void* arg){
	printf("Begin Output Thread\n");
	controller_data_t* controller_data = (controller_data_t*) arg;
	x1_states x1_current = X1_STATE_0;
	while(1){
		sem_wait(&controller_data->sem);
		if(x1_current != controller_data->x1_current_state){
			printf("X1 Updated, current state %d\n", controller_data->x1_current_state);
			x1_current = controller_data->x1_current_state;
		}
		sem_post(&controller_data->sem);
	}
}

int main(void) {
	// Create Global Struct
	controller_data_t global_data;
	c1_global_init(&global_data);

	pthread_t c1_message_thread, output_thread;
	pthread_create(&c1_message_thread, NULL, c1_message_server_thread, &global_data);
	pthread_create(&output_thread, NULL, print_x1_state, &global_data);

	pthread_join(output_thread, NULL);
	pthread_join(c1_message_thread, NULL);

}

/* Controller Message Server Thread - This will handle all messages received from
 * the intersections and railway crossing. This will be responsible for receiving
 * state changes from any node and updating on the controller screen accordingly.
 * Arguments:
 * 	-> controller_data_t
 * @returns
 * 	-> Null
 */
void* c1_message_server_thread(void* arg){
	controller_data_t* controller_data = (controller_data_t*) arg;

	name_attach_t *attach;
	if ((attach = name_attach(NULL, LOCAL_ATTACH_POINT, 0)) == NULL){
		printf("\nFailed to name_attach on: %s \n", LOCAL_ATTACH_POINT);
		printf("\n Possibly another server with the same name is already running !\n");
		return 0;
	}
	printf("Server Listening for Clients on: %s \n", LOCAL_ATTACH_POINT);

	// Setup the sending and receiving message structs
	message_data_t msg;
	message_data_t msg_reply;
	msg_reply.sending_node = NODE_CONTROLLER;
	msg_reply.hdr.type = 0x01;
	msg_reply.hdr.subtype = 0x00;

	int rcvid=0;
	while (1){
		// Receive any message
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

		// Message receive failed
		if (rcvid == -1){
			printf("\nFailed to MsgReceive\n");
			break;
		}

		// Determine the pulse code
		if(rcvid == 0){
			switch(msg.hdr.code){
				case _PULSE_CODE_DISCONNECT:
					printf("_PULSE_CODE_DISCONNECT\n");
					break;
				case _PULSE_CODE_UNBLOCK:
					printf("_PULSE_CODE_UNBLOCK");
					break;
				case _PULSE_CODE_COIDDEATH:
					printf("_PULSE_CODE_COIDDEATH");
					break;
				case _PULSE_CODE_THREADDEATH:
					printf("_PULSE_CODE_THREADDEATH");
					break;
				default:
					printf("Something else received\n");
					break;
			}
		continue;
		}
		if(rcvid > 0){
			//debug printf
			printf("Message received SENDING_NODE:%d TYPE:%d DATA:%d\n", msg.sending_node, msg.msg_type, msg.data);
			msg_reply.receiving_node = msg.sending_node;
			msg_reply.msg_type = msg.msg_type;

			/* TODO
			 * -> Update controller_data_t struct with intersection node state machine enums
			 * ->
			 */
			switch(msg.msg_type){
				case MSG_TRAIN_SIGNAL:
					printf("Signal Message Request\n");
					break;
				case MSG_CURRENT_STATE_REQUEST:
					printf("Current State Request\n");
					break;
				case MSG_CURRENT_STATE_UPDATE:
					// Update sending node current state enum
					// TODO this can definitely be coded better - possibly an array if time permits
					switch(msg.sending_node){
						case NODE_I1:
							// TODO
							break;
						case NODE_I2:
							// TODO
							break;
						case NODE_X1:
							sem_wait(&controller_data->sem);
							controller_data->x1_current_state = msg.data;
							sem_post(&controller_data->sem);
							break;
						default:
							break;
					}
					break;
				default:
					msg_reply.msg_type = MSG_ERROR;
					msg_reply.data = MSG_BAD_REQUEST;
					break;
			}
			MsgReply(rcvid, EOK, &msg_reply, sizeof(msg_reply));
		}
		else{
			printf("\nERROR: Server received something, but could not handle it correctly\n");
		}
	}
	// Remove the attach point name from the file system (i.e. /dev/name/local/<myname>)
	name_detach(attach, 0);
	return (int*) EXIT_FAILURE;
}


/* Function to initialise the controller global struct
 */
void c1_global_init(controller_data_t* data){
	data->x1_current_state = X1_STATE_0;
	// Initialise the global semaphore
	sem_init(&data->sem, 0, 1);
}
