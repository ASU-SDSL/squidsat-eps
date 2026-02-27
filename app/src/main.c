#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include <pb_decode.h>
#include <pb_encode.h>
#include "can_link.h"
#include "proto/eps_link.pb.h"

LOG_MODULE_REGISTER(eps, LOG_LEVEL_INF);

static bool encode_message(uint32_t seq, uint8_t *buffer, size_t *encoded_len)
{
	EpsLinkMessage msg = EpsLinkMessage_init_zero;
	pb_ostream_t stream = pb_ostream_from_buffer(buffer, EpsLinkMessage_size);

	msg.node_id = can_link_node_id();
	msg.seq = seq;
	msg.uptime_ms = k_uptime_get_32();
	msg.type = (seq % 2U == 0U) ? EpsLinkMessage_MsgType_HEARTBEAT : EpsLinkMessage_MsgType_STATUS;

	if (!pb_encode(&stream, EpsLinkMessage_fields, &msg)) {
		LOG_ERR("Nanopb encode failed: %s", PB_GET_ERROR(&stream));
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

static void on_can_message(const uint8_t *buffer, size_t len, void *user_data)
{
	EpsLinkMessage msg;

	ARG_UNUSED(user_data);

	if (!decode_message(buffer, len, &msg)) {
		return;
	}

	LOG_INF("RX node_id=%" PRIu32 " seq=%" PRIu32 " uptime_ms=%" PRIu32 " type=%u",
		msg.node_id, msg.seq, msg.uptime_ms, (unsigned int)msg.type);
}

int main(void)
{
	uint8_t tx_buffer[EpsLinkMessage_size];
	uint32_t seq = 0U;
	int ret;

	ret = can_link_init(on_can_message, NULL);
	if (ret != 0) {
		LOG_ERR("CAN init failed: %d", ret);
		return 0;
	}

	while (1) {
		size_t encoded_len = 0U;

		if (!encode_message(seq, tx_buffer, &encoded_len)) {
			k_msleep(CONFIG_CAN_LINK_TX_PERIOD_MS);
			continue;
		}

		ret = can_link_send(tx_buffer, encoded_len);
		if (ret != 0) {
			LOG_ERR("ISO-TP send failed (seq=%" PRIu32 "): %d", seq, ret);
		} else {
			LOG_INF("TX seq=%" PRIu32 " bytes=%u", seq, (unsigned int)encoded_len);
		}

		seq++;
		k_msleep(CONFIG_CAN_LINK_TX_PERIOD_MS);
	}
}
