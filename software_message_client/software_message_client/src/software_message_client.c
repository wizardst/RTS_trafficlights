#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <global_include.h>
#include <x1_include.h>
#include <pthread.h>

void* sensor_send_thread(void*);

int main(void) {
	printf("\n****** Software Messaging Client for Controller Server ******\n\n");
	pthread_t char_thread;
	pthread_create(&char_thread, NULL, sensor_send_thread, NULL);
	pthread_join(char_thread, NULL);
	return EXIT_SUCCESS;
}

void* sensor_send_thread(void* data){
	message_data_t msg;
	message_data_t reply;
	char* receive_node_name = "";

    msg.sending_node = NODE_I1; // set message signal
    msg.receiving_node = NODE_CONTROLLER;
    msg.msg_type = MSG_CURRENT_STATE_REQUEST;

	// Main Loop
	while(1){
		receive_node_name = "";
		printf("Please select receiving node\n");
		printf(" 0 -> NODE_I1\n 1 -> NODE_I2\n 2 -> NODE_X1\n 3 -> NODE_CONTROLLER\n");
		char x = getchar();
		while(getchar() != '\n');
		switch(x){
			case '0':
				msg.receiving_node = NODE_I1;
				receive_node_name = ""; // TODO
				break;
			case '1':
				msg.receiving_node = NODE_I2;
				receive_node_name = ""; // TODO
				break;
			case '2':
				msg.receiving_node = NODE_X1;
				receive_node_name = X1_QNET_ATTACH_POINT;
				break;
			case '3':
				msg.receiving_node = NODE_CONTROLLER;
				receive_node_name = C1_QNET_ATTACH_POINT;
				break;
			default:
				continue;
				break;
		}

		printf("Please select message type\n");
		printf(" a -> Update Current State\n l -> Send lock state\n r -> Release Lock state\n c -> Request Current State\n"
			   " p -> Request Current Peak Status\n");
		x = getchar();
		while(getchar() != '\n'); // Get remaining characters and discard
		switch(x){
			case 'a':
				printf("Enter the state number\n");
				msg.msg_type = MSG_CURRENT_STATE_UPDATE;
				msg.data = getchar() - 48;
				while(getchar() != '\n');
				reply = send_message(&msg, receive_node_name);
				if(reply.msg_type == MSG_ERROR){
					switch(reply.data){
						case MSG_CONNECTION_ERROR:
							printf("Connection Error!\n");
							break;
						case MSG_SENDING_ERROR:
							printf("Message did not send\n");
							break;
						case MSG_BAD_REQUEST:
							printf("Server rejected this message type\n");
							break;
					}
				}else{
					printf("Success! Received from node %d, data: %d\n", reply.sending_node, reply.data);
				}
				break;

				case 'l':
					msg.msg_type = MSG_CONTROL_STATE_LOCK;
					msg.data = 0;
					reply = send_message(&msg, receive_node_name);
					if(reply.msg_type == MSG_ERROR){
						switch(reply.data){
							case MSG_CONNECTION_ERROR:
								printf("Connection Error!\n");
								break;
							case MSG_SENDING_ERROR:
								printf("Message did not send\n");
								break;
							case MSG_BAD_REQUEST:
								printf("Server rejected this message type\n");
								break;
							case MSG_NO_VALID_RESPONSE:
								printf("Server connected but did not respond\n");
								break;
							default:
								printf("Unknown Error!\n");
								break;
						}
					}else{
						printf("Success! Received from node %d, data: %d\n", reply.sending_node, reply.data);
					}
					break;

				case 'r':
					msg.msg_type = MSG_CONTROL_STATE_RELEASE;
					msg.data = 0;
					reply = send_message(&msg, receive_node_name);
					if(reply.msg_type == MSG_ERROR){
						switch(reply.data){
							case MSG_CONNECTION_ERROR:
								printf("Connection Error!\n");
								break;
							case MSG_SENDING_ERROR:
								printf("Message did not send\n");
								break;
							case MSG_BAD_REQUEST:
								printf("Server rejected this message type\n");
								break;
							case MSG_NO_VALID_RESPONSE:
								printf("Server connected but did not respond\n");
								break;
							default:
								printf("Unkkown Error!\n");
								break;
						}
					}else{
						printf("Success! Received from node %d, data: %d\n", reply.sending_node, reply.data);
					}
					break;

				case 'c':
					msg.msg_type = MSG_CURRENT_STATE_REQUEST;
					msg.data = 0;
					reply = send_message(&msg, receive_node_name);
					if(reply.msg_type == MSG_ERROR){
						switch(reply.data){
							case MSG_CONNECTION_ERROR:
								printf("Connection Error!\n");
								break;
							case MSG_SENDING_ERROR:
								printf("Message did not send\n");
								break;
							case MSG_BAD_REQUEST:
								printf("Server rejected this message type\n");
								break;
							case MSG_NO_VALID_RESPONSE:
								printf("Server connected but did not respond\n");
								break;
							default:
								printf("Unkkown Error!\n");
								break;
						}
					}else{
						printf("Success! Received from node %d, data: %d\n", reply.sending_node, reply.data);
					}
					break;

				case 'p':
					msg.msg_type = MSG_CONTROL_PEAK;
					reply = send_message(&msg, receive_node_name);
					if(reply.msg_type == MSG_ERROR){
						switch(reply.data){
							case MSG_CONNECTION_ERROR:
								printf("Connection Error!\n");
								break;
							case MSG_SENDING_ERROR:
								printf("Message did not send\n");
								break;
							case MSG_BAD_REQUEST:
								printf("Server rejected this message type\n");
								break;
							case MSG_NO_VALID_RESPONSE:
								printf("Server connected but did not respond\n");
								break;
							default:
								printf("Unkkown Error!\n");
								break;
						}
					}else{
						printf("Success! Received from node %d, data: %d\n", reply.sending_node, reply.data);
					}
					break;

				default:
					printf("Invalid option\n\n");
					break;
		}
	}
}
