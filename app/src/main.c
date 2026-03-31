#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <pb_decode.h>
#include <pb_encode.h>
#include "can_link.h"
#include "INA219.h"
#include "proto/eps_link.pb.h"

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
 * @param 	BatState - BatterySate enum from INA219.h, Obtained from INA219_HealthCheck
 * @return 	enum PowerState - Returns a basic enum for now with the power levels listed in the Airtable
 */
enum PowerState ChangePowerState(enum BatteryState BatState){
	switch(BatState){
		case (Battery_START):
			return Nominal;
		case (Battery_OK):
			return Safe;
		case (Battery_LOW):
			return Low_Power;
		default:
			return Nominal;
	}
}

/**
 * @brief:	The goal of this function is to print out the sensor data for the sensors/information
 * 			that we have at the moment. Waiting for EPS Rev 1 to be finished, all code is subject
 * 			to change.
 * @param 	ina219 - Pointer to INA219 instance that we want to get our electrical data from. Only
 * 			variable so far since I'm only aware of the INA219 and no other parts like a 
 * 			thermistor
 */
void GetSensorData(INA219_t *ina219){
	uint16_t current = INA219_ReadCurrent(ina219);
	uint16_t voltage = INA219_ReadBusVoltage(ina219);
	//Temp - whenever Noah finishes the EPS and adds a thermistor if theres not one already

	printf("------------Sensor Data------------");
	printf("Current: %d\nVoltage: %d\nTempurature: ", current, voltage);
	printf("-----------------------------------");
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

	printk("Hello World!\n"); //Just So I dont get a warning when building. Will delete later
	
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

//INA219 instances - names subject to change
	// INA219_t Battery_INA;
	// INA219_t Main_INA;
	// INA219_t Three_Volt_INA;
	// INA219_t Twelve_Volt_INA;
	// INA219_t Three_Volt_Second_INA;
	// INA219_t Five_Volt_INA;
	// INA219_t Solar_Input_INA;
	// INA219_t Five_Volt_RF_INA;

	//I2C typdef
	// I2C_HandleTypeDef hi2c1;
	// I2C_HandleTypeDef hi2c2;
	// I2C_HandleTypeDef hi2c3;
	// I2C_HandleTypeDef hi2c4;
	// I2C_HandleTypeDef hi2c5;
	// I2C_HandleTypeDef hi2c6;
	// I2C_HandleTypeDef hi2c7;
	// I2C_HandleTypeDef hi2c8;

	//INA Initializations
	// while(!INA219_Init(&Battery_INA, &hi2c1, 0x44)){	}
	// while(!INA219_Init(&Main_INA, &hi2c2, 0x40)){}
	// while(!INA219_Init(&Three_Volt_INA, &hi2c3, 0x45)){}
	// while(!INA219_Init(&Twelve_Volt_INA, &hi2c4, 0x49)){}
	// while(!INA219_Init(&Three_Volt_Second_INA, &hi2c5, 0x41)){}
	// while(!INA219_Init(&Five_Volt_INA, &hi2c6, 0x42)){}
	// while(!INA219_Init(&Solar_Input_INA, &hi2c7, 0x43)){}
	// while(!INA219_Init(&Five_Volt_RF_INA, &hi2c8, 0x45)){}

	//Example of using provided funtions. Nothing works yet until we get an actual board working
	// uint16_t power = INA219_ReadPower(&Main_INA);
	// uint16_t current = INA219_ReadCurrent(&Main_INA);
	// float BatteryPercent = INA219_GetBatteryLife(&Battery_INA, 10000, 4000);
	// float BatteryLowThreshold = 25.00;

	// enum BatteryState BatteryLevel = INA219_HealthCheck(&Battery_INA, BatteryLowThreshold, BatteryPercent);

	// ChangePowerState(BatteryLevel);
	// GetSensorData(&Main_INA);

	while (1) {
		size_t encoded_len = 0U;
		struct tx_action action = tx_plan[plan_idx];
		bool encoded_ok;

		if (action.broadcast) {
			encoded_ok = encode_broadcast_message(seq, tx_buffer, &encoded_len);
		} else {
			encoded_ok = encode_unicast_message(seq, tx_buffer, &encoded_len);
		}

		if (!encoded_ok) {
			k_msleep(100);
			continue;
		}

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

		seq++;
		plan_idx = (plan_idx + 1U) % plan_len;
		k_msleep(CONFIG_CAN_LINK_TX_PERIOD_MS);
	}
}
