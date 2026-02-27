#include "can_link.h"

#include <errno.h>
#include <zephyr/canbus/isotp.h>
#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(can_link, LOG_LEVEL_INF);

#define EPS_LOOPBACK_ID 0x18DA55AAU
/*
 * Use dedicated IDs per direction and per flow-control channel.
 * This avoids collisions where one direction's payload frames share the same
 * CAN ID as the opposite direction's FC frames.
 */
#define EPS_A2B_DATA_ID 0x18DA01F1U
#define EPS_B2A_FC_ID 0x18DA02F1U
#define EPS_B2A_DATA_ID 0x18DA03F1U
#define EPS_A2B_FC_ID 0x18DA04F1U

struct eps_isotp_ids {
	struct isotp_msg_id tx_addr;
	struct isotp_msg_id rx_fc_addr;
	struct isotp_msg_id bind_rx_addr;
	struct isotp_msg_id bind_tx_fc_addr;
};

K_THREAD_STACK_DEFINE(rx_thread_stack, CONFIG_CAN_LINK_RX_THREAD_STACK_SIZE);
static struct k_thread rx_thread_data;
K_MUTEX_DEFINE(send_lock);

static struct eps_isotp_ids isotp_ids;
static struct isotp_send_ctx send_ctx;
static struct isotp_recv_ctx recv_ctx;
static const struct isotp_fc_opts fc_opts = {
	.bs = 0,
	.stmin = 0,
};
static const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
static can_link_rx_handler_t rx_callback;
static void *rx_callback_user_data;
static bool is_initialized;

uint32_t can_link_node_id(void)
{
#if defined(CONFIG_EPS_NODE_A)
	return 1U;
#else
	return 2U;
#endif
}

static void fill_isotp_id(struct isotp_msg_id *msg_id, uint32_t ext_id)
{
	*msg_id = (struct isotp_msg_id){
		.ext_id = ext_id,
		.flags = ISOTP_MSG_IDE,
	};
}

static void configure_isotp_ids(struct eps_isotp_ids *ids)
{
	if (IS_ENABLED(CONFIG_CAN_LINK_LOOPBACK)) {
		fill_isotp_id(&ids->tx_addr, EPS_LOOPBACK_ID);
		fill_isotp_id(&ids->rx_fc_addr, EPS_LOOPBACK_ID);
		fill_isotp_id(&ids->bind_rx_addr, EPS_LOOPBACK_ID);
		fill_isotp_id(&ids->bind_tx_fc_addr, EPS_LOOPBACK_ID);
		return;
	}

	if (IS_ENABLED(CONFIG_EPS_NODE_A)) {
		/* A sends A2B_DATA and expects FC on B2A_FC. */
		fill_isotp_id(&ids->tx_addr, EPS_A2B_DATA_ID);
		fill_isotp_id(&ids->rx_fc_addr, EPS_B2A_FC_ID);
		/* A receives B2A_DATA and sends FC on A2B_FC. */
		fill_isotp_id(&ids->bind_rx_addr, EPS_B2A_DATA_ID);
		fill_isotp_id(&ids->bind_tx_fc_addr, EPS_A2B_FC_ID);
	} else {
		/* B sends B2A_DATA and expects FC on A2B_FC. */
		fill_isotp_id(&ids->tx_addr, EPS_B2A_DATA_ID);
		fill_isotp_id(&ids->rx_fc_addr, EPS_A2B_FC_ID);
		/* B receives A2B_DATA and sends FC on B2A_FC. */
		fill_isotp_id(&ids->bind_rx_addr, EPS_A2B_DATA_ID);
		fill_isotp_id(&ids->bind_tx_fc_addr, EPS_B2A_FC_ID);
	}
}

static void rx_thread(void *arg1, void *arg2, void *arg3)
{
	static uint8_t rx_buffer[CONFIG_CAN_LINK_RX_BUFFER_SIZE];
	int ret;

	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	ret = isotp_bind(&recv_ctx, can_dev, &isotp_ids.bind_rx_addr, &isotp_ids.bind_tx_fc_addr,
			 &fc_opts, K_FOREVER);
	if (ret != ISOTP_N_OK) {
		LOG_ERR("ISO-TP bind failed: %d", ret);
		return;
	}

	LOG_INF("ISO-TP RX ready on ID 0x%08x", isotp_ids.bind_rx_addr.ext_id);

	while (1) {
		int rx_len = isotp_recv(&recv_ctx, rx_buffer, sizeof(rx_buffer),
					K_MSEC(CONFIG_CAN_LINK_RX_TIMEOUT_MS));

		if (rx_len == ISOTP_RECV_TIMEOUT) {
			continue;
		}

		if (rx_len < 0) {
			LOG_ERR("ISO-TP receive failed: %d", rx_len);
			continue;
		}

		if (rx_callback != NULL) {
			rx_callback(rx_buffer, (size_t)rx_len, rx_callback_user_data);
		}
	}
}

int can_link_init(can_link_rx_handler_t rx_handler, void *user_data)
{
	k_tid_t rx_tid;
	can_mode_t mode;
	int ret;

	if (is_initialized) {
		return 0;
	}

	if (!device_is_ready(can_dev)) {
		LOG_ERR("CAN device not ready: %s", can_dev->name);
		return -ENODEV;
	}

	configure_isotp_ids(&isotp_ids);
	mode = IS_ENABLED(CONFIG_CAN_LINK_LOOPBACK) ? CAN_MODE_LOOPBACK : CAN_MODE_NORMAL;

	ret = can_set_mode(can_dev, mode);
	if (ret != 0) {
		LOG_ERR("Failed to set CAN mode: %d", ret);
		return ret;
	}

	ret = can_start(can_dev);
	if (ret != 0) {
		LOG_ERR("Failed to start CAN controller: %d", ret);
		return ret;
	}

	rx_callback = rx_handler;
	rx_callback_user_data = user_data;

	LOG_INF("CAN start board=%s node=%u loopback=%d", CONFIG_BOARD, can_link_node_id(),
		IS_ENABLED(CONFIG_CAN_LINK_LOOPBACK));
	LOG_INF("ISO-TP TX ID 0x%08x, RX/FC ID 0x%08x",
		isotp_ids.tx_addr.ext_id, isotp_ids.rx_fc_addr.ext_id);

	rx_tid = k_thread_create(&rx_thread_data, rx_thread_stack,
				 K_THREAD_STACK_SIZEOF(rx_thread_stack), rx_thread,
				 NULL, NULL, NULL, CONFIG_CAN_LINK_RX_THREAD_PRIORITY, 0,
				 K_NO_WAIT);
	if (rx_tid == NULL) {
		LOG_ERR("Failed to create RX thread");
		return -EIO;
	}
	k_thread_name_set(rx_tid, "can_rx");
	is_initialized = true;

	return 0;
}

int can_link_send(const uint8_t *payload, size_t len)
{
	int ret;

	if (!is_initialized) {
		return -EACCES;
	}

	if ((payload == NULL) || (len == 0U)) {
		return -EINVAL;
	}

	k_mutex_lock(&send_lock, K_FOREVER);
	ret = isotp_send(&send_ctx, can_dev, payload, len, &isotp_ids.tx_addr, &isotp_ids.rx_fc_addr,
			 NULL, NULL);
	k_mutex_unlock(&send_lock);

	return ret;
}
