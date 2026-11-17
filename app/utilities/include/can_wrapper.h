#ifndef CAN_WRAPPER_H
#define CAN_WRAPPER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <zephyr/canbus/isotp.h>
#include <zephyr/device.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include "can_link.h"
#include "ina219.h"
#include "proto/eps_link.pb.h"

struct tx_action {
	bool broadcast;
	uint8_t target_node;
};

extern uint32_t seq;
extern size_t plan_len;
extern size_t plan_idx;
extern int ret;
extern struct tx_action tx_plan[3];

bool encode_unicast_message(uint32_t seq, uint8_t *buffer, size_t *encoded_len, ina219_data_t *inaData);
bool encode_broadcast_message(uint32_t seq, uint8_t *buffer, size_t *encoded_len);
bool decode_message(const uint8_t *buffer, size_t len, EpsLinkMessage *msg);
size_t build_tx_plan(uint8_t local_node, struct tx_action *plan, size_t max_actions);
void on_can_message(const uint8_t *buffer, size_t len, uint8_t source_node, bool is_broadcast, void *user_data);

#endif