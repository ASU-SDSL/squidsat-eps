#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/devicetree.h>

#include <pb_decode.h>
#include <pb_encode.h>
#include "can_link.h"
#include "proto/eps_link.pb.h"
#include "sensor_task.h"
#include "nka103c1b1.h"
#include "ntcle101.h" 

#define ADC_NODE DT_PATH(zephyr_user)
static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET_BY_IDX(ADC_NODE, 0);

K_THREAD_STACK_DEFINE(sensor_task_stack, 1024);
struct k_thread sensor_task_thread;

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


//MAIN FUNCTION
int main(void)
{
	uint8_t tx_buffer[EpsLinkMessage_size];
	struct tx_action tx_plan[3];
	uint32_t seq = 0U;
	size_t plan_len;
	size_t plan_idx = 0U;
	int ret;

	
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

	//ADC Code for the EPS Thermistor
	int fault;
    uint32_t raw_value = 0;

    if (!adc_is_ready_dt(&adc_channel)) {
        //LOG_ERR("ADC controller not ready\n");
        return 0;
    }

    fault = adc_channel_setup_dt(&adc_channel);
    if (fault < 0) {
        //LOG_ERR("Could not setup channel (%d)\n", fault);
        return 0;
    }

    struct adc_sequence sequence = {
        .channels = BIT(adc_channel.channel_id),
        .buffer = &raw_value,
        .buffer_size = sizeof(raw_value),
        .resolution = adc_channel.resolution,
    };
	// END ADC CODE

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

		fault = adc_read_dt(&adc_channel, &sequence);
    	if (fault < 0) {
        	//LOG_ERR("Could not read (%d)\n", fault);
        	return 0;
    	} 
		
		// float temp = getBattTemp(raw_value);
		// printk("Batt temperature is: %f", temp);

		k_msleep(CONFIG_CAN_LINK_TX_PERIOD_MS);
	}
}
