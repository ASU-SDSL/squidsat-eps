#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#include <pb_decode.h>
#include <pb_encode.h>
#include "state.h"
#include "proto/eps_link.pb.h"
#include "can_link.h"
#include "sensor_task.h"
#include "nka103c1b1.h"
#include "ntcle101.h" 
#include "ntcle101.h"
#include "ina219.h"



K_THREAD_STACK_DEFINE(sensor_task_stack, 1024);		// Creating the sensor__task
struct k_thread sensor_task_thread;

LOG_MODULE_REGISTER(eps, LOG_LEVEL_INF);



// MAIN FUNCTION
int main(void) {
	//printk("Aidan was here");
	uint8_t tx_buffer[EpsLinkMessage_size];

	ina219_data_t inaData = {0};
	
	/* Set the node ID in can_link.c to be 0x03. This is the node address for the EPS 
	This sets the plan length to be 2, and the action.broadcat to be false (meaning we want to target one node like the OBC)
	Having the action.broadcast be false, sets the code to use the unicast functions below in each if statement */
	ret = can_link_init(on_can_message, NULL);
	plan_len = build_tx_plan((uint8_t)can_link_node_id(), tx_plan, ARRAY_SIZE(tx_plan));

	// Sensor_task_code
	void sensor_task_entry(void *p1, void *p2, void *p3){
    	ARG_UNUSED(p1);
    	ARG_UNUSED(p2);
    	ARG_UNUSED(p3);
    	sensor_task();
	}

	k_thread_create(&sensor_task_thread, 
					sensor_task_stack,
                    K_THREAD_STACK_SIZEOF(sensor_task_stack),
                    sensor_task_entry, 
					NULL, 
					NULL, 
					NULL,
                    5, 
					0, 
					K_NO_WAIT);
	// END SENSOR TASK CODE

	// WAYNES CAN CODE - INIT (I think)
	if (ret != 0) {
		LOG_ERR("CAN init failed: %d", ret);
		return 0;
	}

	if (plan_len == 0U) {
		LOG_ERR("Failed to build TX plan");
		return 0;
	}

	LOG_INF("TX plan loaded for node=%u entries=%u", (unsigned int)can_link_node_id(), (unsigned int)plan_len);
	// END CAN INIT

	
	// TODO: Perhaps test for a single, or make a new temporary ina instance to initialize here instead of every ina read		
	readSingleINA(0, &inaData);
	printk("INA data is: %fV, %fA, %fW", (double)inaData.voltage, (double)inaData.current, (double)inaData.power);
	
	size_t encoded_len = 0U;
	struct tx_action action = tx_plan[plan_idx];
	bool encoded_ok;

	// tx_buffer is the data we want to transmit, split into 1 byte (8bit) sections
	// we must follow the format of the eps_link.proto file when adding data to the buffer
	tx_buffer[26] = inaData.current;
	tx_buffer[30] = inaData.voltage;
	tx_buffer[34] = inaData.power;
	tx_buffer[38] = inaData.shuntVoltage;
	//action.broadcast = false;				// Send to single node (OBC or DANT, not to everyone)

	if (action.broadcast) {
		encoded_ok = encode_broadcast_message(seq, tx_buffer, &encoded_len);
	} else {
		encoded_ok = encode_unicast_message(seq, tx_buffer, &encoded_len);	// Encode Protobuf message (? Comment by Aidan)
	}

	

	if (!encoded_ok) {
		k_msleep(100);
	}

	// If TX wants to send a broadcast
	if (action.broadcast) {
		// Try and sent the tx_buffer to another CAN transciever
		ret = can_link_send_broadcast(tx_buffer, encoded_len);

		if (ret != 0) {
			LOG_ERR("TX broadcast failed (seq=%" PRIu32 "): %d", seq, ret);
		} else {
			LOG_INF("TX broadcast seq=%" PRIu32 " bytes=%u", seq, (unsigned int)encoded_len);
		}
	} else {
		// Try and send tx_buffer to specific node (? idk)
		ret = can_link_send_to(action.target_node, tx_buffer, encoded_len);

		if (ret != 0) {
			LOG_ERR("TX unicast failed (seq=%" PRIu32 ", target=%u): %d", seq, action.target_node, ret);
		} else {
			LOG_INF("TX unicast seq=%" PRIu32 " target=%u bytes=%u", seq, action.target_node, (unsigned int)encoded_len);
		}
	}

	seq++;										// Increase attempt number (? Comment written by Aidan)
	plan_idx = (plan_idx + 1U) % plan_len;		// Try the other plans in tx_plan. Should all be false (unicast) for EPS. Tries to send to OBC then DANT

	//float temp = getBattTemp(raw_value);
	//printk("Batt temperature is: %f", temp);

	k_msleep(CONFIG_CAN_LINK_TX_PERIOD_MS);
}
