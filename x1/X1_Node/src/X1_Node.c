#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Global includes
#include <data_types.h>
#include <x1_include.h>

// Func prototypes
void* sensor_send_thread(void*);
void* x1_state_machine(void*);
void* x1_state_machine_outputs(void*);

int main(void) {
	printf("Begin X1 State Machine Node\n");

	// Initialise global variable
	sm_data_t global_data;
	global_data.current_state = X1_STATE_0;
	global_data.sensor_received = 0; // Ensure this is zero
	global_data.signal = 0;
	// Initialise the global semaphore
	sem_init(&global_data.sem, 0, 1);

	// Create the threads
	pthread_t sensor_thread, state_machine_thread, state_machine_output_thread;
	pthread_create(&sensor_thread, NULL, sensor_send_thread, &global_data);
	pthread_create(&state_machine_thread, NULL, x1_state_machine, &global_data);
	pthread_create(&state_machine_output_thread, NULL, x1_state_machine_outputs, &global_data);

	// Join on all threads (should never return a value however should handle anyway)
	pthread_join(sensor_thread, NULL);
	pthread_join(state_machine_thread, NULL);
	pthread_join(state_machine_output_thread, NULL);

	printf("State Machine Node Ended\n");
	return EXIT_SUCCESS;
}


/* State machine output thread used to handle the outputs of the state machine
 * -> This wil be converted to hardware when implemented on the beaglebone to output
 * 	  to the LCD
 */
void* x1_state_machine_outputs(void* arg){
	// Cast the global struct
	sm_data_t* msg = (sm_data_t*) arg;

	// Last state variable so that the print statements don't spam the console (should
	// probably remove this on hardware..?)
	sem_wait(&msg->sem); // Protect Data
	x1_states last_state = msg->current_state;
	sem_post(&msg->sem); // Release

	while(1){
		sem_wait(&msg->sem);
		if(last_state != msg->current_state){
			switch(msg->current_state){
				case X1_STATE_0:
					printf("X1_STATE_0\n");
					break;
				case X1_STATE_1:
					printf("X1_STATE_1\n");
					break;
				case X1_STATE_2:
					printf("X1_STATE_2\n");
					break;
				case X1_STATE_3:
					printf("X1_STATE_3\n");
					break;
				case X1_STATE_4:
					printf("X1_STATE_4\n");
					break;
				case X1_STATE_5:
					printf("X1_STATE_5\n");
					break;
				case X1_STATE_6:
					printf("X1_STATE_6\n");
					break;
				case X1_STATE_7:
					printf("X1_STATE_7\n");
					break;
			}
		// Update the last state variable
		last_state = msg->current_state;
		}
		sem_post(&msg->sem);
	}
}

/* State machine input thread
 * -> This will handle the input variables and determine the next state conditions
 */
void* x1_state_machine(void* arg){
	// Cast the global struct
	sm_data_t* msg = (sm_data_t*) arg;
	while(1){
		sem_wait(&msg->sem);
		switch(msg->current_state){
			case X1_STATE_0:

				// Logic for next state
				if(msg->sensor_received){
					if((msg->signal & 1 << X1_SIGNAL_Ein) && (msg->signal & 1 << X1_SIGNAL_Win)){ // E&W signal
						msg->current_state = X1_STATE_3;
					}else if(msg->signal & 1 << X1_SIGNAL_Ein){ // Ein Signal
						msg->current_state = X1_STATE_1;
					}else if(msg->signal & 1 << X1_SIGNAL_Win){ // Win Signal
						msg->current_state = X1_STATE_2;
					}
				}
				break;

			case X1_STATE_1:

				// Logic for next state
				if(msg->sensor_received){
					if(msg->signal & 1 << X1_SIGNAL_Win){ // Win - second train detected
						msg->current_state = X1_STATE_3;
					}else if(msg->signal & 1 << X1_SIGNAL_BoomGateDown){
						msg->current_state = X1_STATE_4; // Boom gate is down
					}
				}
				break;
			case X1_STATE_2:

				// Logic for next state
				if(msg->sensor_received){
					if(msg->signal & 1 << X1_SIGNAL_Ein){ // Ein - second train detected
						msg->current_state = X1_STATE_3;
					}else if(msg->signal & 1 << X1_SIGNAL_BoomGateDown){
						msg->current_state = X1_STATE_5; // Boom gate is down
					}
				}
				break;
			case X1_STATE_3:

				// Logic for next state
				if(msg->sensor_received){
					if(msg->signal & 1 << X1_SIGNAL_BoomGateDown){ // Ein - second train detected
						msg->current_state = X1_STATE_6;
					}
				}
				break;
			case X1_STATE_4:

				// Logic for next state
				if(msg->sensor_received){
					if(msg->signal & 1 << X1_SIGNAL_Win){ // Win - second train detected
						msg->current_state = X1_STATE_6;
					}else if(msg->signal & 1 << X1_SIGNAL_Eout){// Eout - Train clear of crossing
						msg->current_state = X1_STATE_7;
					}
				}
				break;
			case X1_STATE_5:

				// Logic for next state
				if(msg->sensor_received){
					if(msg->signal & 1 << X1_SIGNAL_Ein){ // Win - second train detected
						msg->current_state = X1_STATE_6;
					}else if(msg->signal & 1 << X1_SIGNAL_Wout){// Eout - Train clear of crossing
						msg->current_state = X1_STATE_7;
					}
				}
				break;
			case X1_STATE_6:

				// Logic for next state
				if(msg->sensor_received){
					if((msg->signal & 1 << X1_SIGNAL_Wout) && (msg->signal & 1 << X1_SIGNAL_Eout)){ // Win - second train detected
						msg->current_state = X1_STATE_7;
					}

					// Only needed for sensor thread, but will still work anyway
					else if(msg->signal & 1 << X1_SIGNAL_Wout){
						msg->current_state = X1_STATE_4;
					}else if(msg->signal & 1 << X1_SIGNAL_Eout){
						msg->current_state = X1_STATE_5;
					}
				}
				break;
			case X1_STATE_7:
				// Logic for next state
				if(msg->sensor_received){
					if((msg->signal & 1 << X1_SIGNAL_Ein) && (msg->signal & 1 << X1_SIGNAL_Win)){ // E&W signal
						msg->current_state = X1_STATE_6;
					}else if(msg->signal & 1 << X1_SIGNAL_Ein){ // Ein Signal
						msg->current_state = X1_STATE_4;
					}else if(msg->signal & 1 << X1_SIGNAL_Win){ // Win Signal
						msg->current_state = X1_STATE_5;
					}else if(~msg->signal & 1 << X1_SIGNAL_BoomGateDown){
						msg->current_state = X1_STATE_0;
					}
				}
				break;
			default:
				printf("State machine error\n");
				msg->current_state = X1_STATE_0;
				break;
		}
		msg->sensor_received = 0;
		sem_post(&msg->sem);
	}
}

/*
 * Thread to get char and send to x1 node. (will be replaced by a hardware thread)
 *
 */
void* sensor_send_thread(void* data){
	sm_data_t* sens_data = (sm_data_t*) data;

	// Main Loop
	while(1){
		char x = getchar();
		getchar(); // get the newline

		// Clear the sensor data except the latching boomgate sensor (only needed for sensor thread)
		sem_wait(&sens_data->sem);
		sens_data->signal &= (1 << X1_SIGNAL_BoomGateDown);
		sem_post(&sens_data->sem);

		switch(x){
			// East approching signal
			case 'e':

				sem_wait(&sens_data->sem);
				sens_data->sensor_received = 1;
				sens_data->signal |= 1 << X1_SIGNAL_Ein;
				sem_post(&sens_data->sem);

				printf("Train Approaching from the East\n");
				break;

			// East Leaving signal
			case 'd':

				sem_wait(&sens_data->sem);
				sens_data->sensor_received = 1;
				sens_data->signal |= 1 << X1_SIGNAL_Eout;
				sem_post(&sens_data->sem);

				printf("Train clear on the East line\n");
				break;
			// West Approaching signal
			case 'w':

				sem_wait(&sens_data->sem);
				sens_data->sensor_received = 1;
				sens_data->signal |= 1 << X1_SIGNAL_Win;
				sem_post(&sens_data->sem);

				printf("Train Approaching from the West\n");
				break;
			// West Departing signal
			case 's':

				sem_wait(&sens_data->sem);
				sens_data->sensor_received = 1;
				sens_data->signal |= 1 << X1_SIGNAL_Wout;
				sem_post(&sens_data->sem);

				printf("Train clear on the West line\n");
				break;
			// Boom gate down signal
			case 'b':

				sem_wait(&sens_data->sem);
				sens_data->sensor_received = 1;
				sens_data->signal |= 1 << X1_SIGNAL_BoomGateDown;
				sem_post(&sens_data->sem);

				printf("Boom gate down\n");
				break;
			// Boom gate up signal
			case 'u':

				sem_wait(&sens_data->sem);
				sens_data->sensor_received = 1;
				sens_data->signal &= ~(1 << X1_SIGNAL_BoomGateDown);
				sem_post(&sens_data->sem);

				printf("Boom gate up\n");
				break;
			default:
				printf("Unrecognised character, please try again\n");
				break;
		}
	}
}
