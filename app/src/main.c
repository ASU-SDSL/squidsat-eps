#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/sensor.h>

#include <pb_decode.h>
#include <pb_encode.h>
#include "can_link.h"
#include "proto/eps_link.pb.h"

#define INA_MAIN DT_NODELABEL(ina219_0)

LOG_MODULE_REGISTER(eps, LOG_LEVEL_INF);

struct tx_action {
	bool broadcast;
	uint8_t target_node;
};

static bool encode_unicast_message(uint32_t seq, uint8_t *buffer, size_t *encoded_len)
{
	EpsLinkMessage msg = EpsLinkMessage_init_zero;
	pb_ostream_t stream = pb_ostream_from_buffer(buffer, EpsLinkMessage_size);

	msg.node_id = can_link_node_id();
	msg.seq = seq;
	msg.type = (seq % 2U == 0U) ? EpsLinkMessage_MsgType_HEARTBEAT : EpsLinkMessage_MsgType_STATUS;

	if (!pb_encode(&stream, EpsLinkMessage_fields, &msg)) {
		LOG_ERR("Nanopb encode failed: %s", PB_GET_ERROR(&stream));
		return false;
	}

	*encoded_len = stream.bytes_written;
	return true;
}

static bool encode_broadcast_message(uint32_t seq, uint8_t *buffer, size_t *encoded_len)
{
	EpsLinkMessage msg = EpsLinkMessage_init_zero;
	pb_ostream_t stream = pb_ostream_from_buffer(buffer, EpsLinkMessage_size);

	msg.node_id = can_link_node_id();
	msg.seq = seq;
	msg.type = EpsLinkMessage_MsgType_STATUS;

	if (!pb_encode(&stream, EpsLinkMessage_fields, &msg)) {
		LOG_ERR("Nanopb broadcast encode failed: %s", PB_GET_ERROR(&stream));
		return false;
	}

	*encoded_len = stream.bytes_written;
	return true;
}

static bool decode_message(const uint8_t *buffer, size_t len, EpsLinkMessage *msg)
{
	pb_istream_t stream = pb_istream_from_buffer(buffer, len);

	*msg = (EpsLinkMessage)EpsLinkMessage_init_zero;
	if (!pb_decode(&stream, EpsLinkMessage_fields, msg)) {
		LOG_ERR("Nanopb decode failed: %s", PB_GET_ERROR(&stream));
		return false;
	}

	return true;
}

static void on_can_message(const uint8_t *buffer, size_t len, uint8_t source_node, bool is_broadcast,
			   void *user_data)
{
	EpsLinkMessage msg;

	ARG_UNUSED(user_data);

	if (!decode_message(buffer, len, &msg)) {
		return;
	}

	LOG_INF("RX src=%u broadcast=%d node_id=%" PRIu32 " seq=%" PRIu32 " uptime_ms=%" PRIu32 " type=%u",
		source_node, is_broadcast, msg.node_id, msg.seq, msg.uptime_ms, (unsigned int)msg.type);
}

static size_t build_tx_plan(uint8_t local_node, struct tx_action *plan, size_t max_actions)
{
	if (max_actions < 3U) {
		return 0U;
	}

	if (local_node == 0x01U) {
		plan[0] = (struct tx_action){ .broadcast = true, .target_node = 0xFFU };
		plan[1] = (struct tx_action){ .broadcast = false, .target_node = 0x02U };
		plan[2] = (struct tx_action){ .broadcast = false, .target_node = 0x03U };
		return 3U;
	}

	if (local_node == 0x02U) {
		plan[0] = (struct tx_action){ .broadcast = false, .target_node = 0x01U };
		plan[1] = (struct tx_action){ .broadcast = false, .target_node = 0x03U };
		return 2U;
	}

	if (local_node == 0x03U) {
		plan[0] = (struct tx_action){ .broadcast = false, .target_node = 0x01U };
		plan[1] = (struct tx_action){ .broadcast = false, .target_node = 0x02U };
		return 2U;
	}

	plan[0] = (struct tx_action){ .broadcast = false, .target_node = (uint8_t)CONFIG_CAN_LINK_PEER_NODE_ADDR };
	return 1U;
}

/**
 * @brief:	The goal of this function is to change the power mode of the EPS board. The buit in INA 
 * 			file may be enough, but I dont know for now. BAREONES CODE until flatsat/mcu provided for.
 * 			testing as well as more detailed requirements. All code is subject to change 
 * @param 	value - gets the struct of the value used to determine power state. value.val1 is the whole
 * 			number, while value.val2 (if used) is the decimal
 * @return 	enum PowerState - Returns a basic enum for now with the power levels listed in the Airtable
 */
// enum PowerState ChangePowerState(struct sensor_value value){
//  //Just an example, these values dont mean or do anything yet
// 	switch(value.val1){
// 		case (5):
// 			return Nominal;
// 		case (3):
// 			return Safe;
// 		case (2.9):
// 			return Low_Power;
// 		default:
// 			return Nominal;
// 	}
// }

/**
 * @brief:	The goal of this function is to print out the sensor data for the sensors/information
 * 			that we have at the moment. Waiting for EPS Rev 1 to be finished, all code is subject
 * 			to change.
 * @param 	ina - Pointer to INA219 instance that we want to get our electrical data from. 
 * @param   type - Zephyr Sensor enum. For the INA all we need is SENSOR_CHAN_VOLTAGE, SENSOR_CHAN_CURRENT, and
 * 			SENSOR_CHAN_POWER. Shunt voltage can be obtained the same way if needed.
 * @param   val - struct used to store the values obtained from INA219 register. val.val1 is the whole number and
 *          val.val2 is the decimal.
 */
int GetSensorData(const struct device *ina, struct sensor_value voltage, struct sensor_value current, struct sensor_value power){
	int init = sensor_sample_fetch(ina);
	if (init) {
		printf("Could not fetch sensor data.\n");
		LOG_ERR("Could not fetch sensor data.\n");
		return 0;
	}

	sensor_channel_get(ina, SENSOR_CHAN_VOLTAGE, &voltage);
	sensor_channel_get(ina, SENSOR_CHAN_POWER, &current);
	sensor_channel_get(ina, SENSOR_CHAN_CURRENT, &power);

	printf("------------------------Sensor Data---------------------\nVoltage is: %fV, Current is: %fA, Power is: %fW\n--------------------------------------------------------\n", 
		   sensor_value_to_double(&voltage),sensor_value_to_double(&current),sensor_value_to_double(&power));
	
	LOG_INF("------------------------Sensor Data---------------------\nVoltage is: %fV, Current is: %fA, Power is: %fW\n--------------------------------------------------------\n", 
		   sensor_value_to_double(&voltage),sensor_value_to_double(&current),sensor_value_to_double(&power));
	
	
	return 1;
}

//MAIN FUNCTION
int main(void)
{
	uint8_t tx_buffer[EpsLinkMessage_size];
	struct tx_action tx_plan[3];
	uint32_t seq = 0U;
	size_t plan_len;
	size_t plan_idx = 0U;
	int ret;

	//printk("Hello World!\n"); //Just So I dont get a warning when building. Will delete later
	
	ret = can_link_init(on_can_message, NULL);
	if (ret != 0) {
		LOG_ERR("CAN init failed: %d", ret);
		return 0;
	}

	plan_len = build_tx_plan((uint8_t)can_link_node_id(), tx_plan, ARRAY_SIZE(tx_plan));
	if (plan_len == 0U) {
		LOG_ERR("Failed to build TX plan");
		return 0;
	}

	LOG_INF("TX plan loaded for node=%u entries=%u", (unsigned int)can_link_node_id(),
		(unsigned int)plan_len);

	//New intatiation of a test INA struct based off the Zephyr INA219 API. Ignore the error squiggles
	const struct device *inaTest = DEVICE_DT_GET(INA_MAIN);
	struct sensor_value voltage, current, power;

	while (1) {
		size_t encoded_len = 0U;
		struct tx_action action = tx_plan[plan_idx];
		bool encoded_ok;

		//Testing that the INA219 breakout board can be read and my function works - Aidan Doyle
		if (!device_is_ready(inaTest)) {
			printf("Device %s is not ready.\n", inaTest->name);
			LOG_ERR("Device %s is not ready.\n", inaTest->name);
		}else{
			printf("Getting Data...");
			LOG_INF("Getting Data...");
			GetSensorData(inaTest, voltage, current, power);
		}

		if (action.broadcast) {
			encoded_ok = encode_broadcast_message(seq, tx_buffer, &encoded_len);
		} else {
			encoded_ok = encode_unicast_message(seq, tx_buffer, &encoded_len);
		}

		if (!encoded_ok) {
			k_msleep(100);
			continue;
		}

		if(seq < 2){ //Just for debugging, so the seriel monitor doesnt get spammed with can errors - Aidan
		if (action.broadcast) {
			ret = can_link_send_broadcast(tx_buffer, encoded_len);
			if (ret != 0) {
				LOG_ERR("TX broadcast failed (seq=%" PRIu32 "): %d", seq, ret);
			} else {
				LOG_INF("TX broadcast seq=%" PRIu32 " bytes=%u", seq, (unsigned int)encoded_len);
			}
		} else {
			ret = can_link_send_to(action.target_node, tx_buffer, encoded_len);
			if (ret != 0) {
				LOG_ERR("TX unicast failed (seq=%" PRIu32 ", target=%u): %d", seq,
					action.target_node, ret);
			} else {
				LOG_INF("TX unicast seq=%" PRIu32 " target=%u bytes=%u", seq,
					action.target_node, (unsigned int)encoded_len);
			}
		}
		}
		seq++;
		plan_idx = (plan_idx + 1U) % plan_len;
		k_msleep(CONFIG_CAN_LINK_TX_PERIOD_MS);
	}
}
