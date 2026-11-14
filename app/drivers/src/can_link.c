#include "can_link.h"

#include <errno.h>
#include <zephyr/canbus/isotp.h>
#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(can_link, LOG_LEVEL_INF);

#define EPS_FIXED_TATYPE_PHYSICAL 0xDAU
#define EPS_FIXED_TATYPE_FUNCTIONAL 0xDBU
#define EPS_BROADCAST_NODE 0xFFU

struct eps_isotp_ids {
	struct isotp_msg_id tx_addr;
	struct isotp_msg_id rx_fc_addr;
	struct isotp_msg_id bind_rx_addr;
	struct isotp_msg_id bind_tx_fc_addr;
	struct isotp_msg_id broadcast_tx_addr;
	struct isotp_msg_id broadcast_bind_rx_addr;
	struct isotp_msg_id broadcast_bind_tx_addr;
};

struct rx_worker {
	struct isotp_recv_ctx *ctx;
	const struct isotp_msg_id *rx_addr;
	const struct isotp_msg_id *tx_addr;
	bool is_broadcast;
};

K_THREAD_STACK_DEFINE(unicast_rx_thread_stack, CONFIG_CAN_LINK_RX_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(broadcast_rx_thread_stack, CONFIG_CAN_LINK_RX_THREAD_STACK_SIZE);
static struct k_thread unicast_rx_thread_data;
static struct k_thread broadcast_rx_thread_data;
K_MUTEX_DEFINE(send_lock);

static struct eps_isotp_ids isotp_ids;
static struct isotp_send_ctx send_ctx;
static struct isotp_send_ctx broadcast_send_ctx;
static struct isotp_recv_ctx recv_ctx;
static struct isotp_recv_ctx broadcast_recv_ctx;
static const struct isotp_fc_opts fc_opts = {
	.bs = 0,
	.stmin = 0,
};
static const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
static can_link_rx_handler_t rx_callback;
static void *rx_callback_user_data;
static bool is_initialized;

static struct rx_worker unicast_worker = {
	.ctx = &recv_ctx,
	.rx_addr = &isotp_ids.bind_rx_addr,
	.tx_addr = &isotp_ids.bind_tx_fc_addr,
	.is_broadcast = false,
};

static struct rx_worker broadcast_worker = {
	.ctx = &broadcast_recv_ctx,
	.rx_addr = &isotp_ids.broadcast_bind_rx_addr,
	.tx_addr = &isotp_ids.broadcast_bind_tx_addr,
	.is_broadcast = true,
};

uint32_t can_link_node_id(void)
{
	return (uint32_t)CONFIG_CAN_LINK_NODE_ADDR;
}

static uint32_t build_fixed_addr_id(uint8_t priority, uint8_t ta_type, uint8_t target, uint8_t source)
{
	uint32_t id = 0U;

	id |= (((uint32_t)priority << ISOTP_FIXED_ADDR_PRIO_POS) & ISOTP_FIXED_ADDR_PRIO_MASK);
	id |= (((uint32_t)target << ISOTP_FIXED_ADDR_TA_POS) & ISOTP_FIXED_ADDR_TA_MASK);
	id |= (((uint32_t)source << ISOTP_FIXED_ADDR_SA_POS) & ISOTP_FIXED_ADDR_SA_MASK);
	id |= ((uint32_t)ta_type << 16);

	return id;
}

static uint8_t extract_source_node(const struct isotp_recv_ctx *ctx)
{
	return (uint8_t)((ctx->rx_addr.ext_id & ISOTP_FIXED_ADDR_SA_MASK) >> ISOTP_FIXED_ADDR_SA_POS);
}

static void fill_isotp_fixed_id(struct isotp_msg_id *msg_id, uint8_t priority, uint8_t ta_type,
			       uint8_t target, uint8_t source)
{
	*msg_id = (struct isotp_msg_id){
		.ext_id = build_fixed_addr_id(priority, ta_type, target, source),
		.flags = ISOTP_MSG_FIXED_ADDR | ISOTP_MSG_IDE,
	};
}

static void configure_isotp_ids(struct eps_isotp_ids *ids)
{
	const uint8_t local = (uint8_t)CONFIG_CAN_LINK_NODE_ADDR;
	const uint8_t peer = IS_ENABLED(CONFIG_CAN_LINK_LOOPBACK) ?
				     local :
				     (uint8_t)CONFIG_CAN_LINK_PEER_NODE_ADDR;

	/* Unicast: local -> peer data, peer -> local flow-control. */
	fill_isotp_fixed_id(&ids->tx_addr, CONFIG_CAN_LINK_TELEMETRY_PRIORITY,
			   EPS_FIXED_TATYPE_PHYSICAL, peer, local);
	fill_isotp_fixed_id(&ids->rx_fc_addr, CONFIG_CAN_LINK_COMMAND_PRIORITY,
			   EPS_FIXED_TATYPE_PHYSICAL, local, peer);

	/* Unicast receive: accept physical frames addressed to local from any source. */
	fill_isotp_fixed_id(&ids->bind_rx_addr, CONFIG_CAN_LINK_COMMAND_PRIORITY,
			   EPS_FIXED_TATYPE_PHYSICAL, local, 0U);
	fill_isotp_fixed_id(&ids->bind_tx_fc_addr, CONFIG_CAN_LINK_COMMAND_PRIORITY,
			   EPS_FIXED_TATYPE_PHYSICAL, 0U, local);

	/* Broadcast (functional): sender targets 0xFF. */
	fill_isotp_fixed_id(&ids->broadcast_tx_addr, CONFIG_CAN_LINK_COMMAND_PRIORITY,
			   EPS_FIXED_TATYPE_FUNCTIONAL, EPS_BROADCAST_NODE, local);
	fill_isotp_fixed_id(&ids->broadcast_bind_rx_addr, CONFIG_CAN_LINK_COMMAND_PRIORITY,
			   EPS_FIXED_TATYPE_FUNCTIONAL, EPS_BROADCAST_NODE, 0U);
	fill_isotp_fixed_id(&ids->broadcast_bind_tx_addr, CONFIG_CAN_LINK_COMMAND_PRIORITY,
			   EPS_FIXED_TATYPE_FUNCTIONAL, 0U, local);
}

static void rx_thread(void *arg1, void *arg2, void *arg3)
{
	uint8_t rx_buffer[CONFIG_CAN_LINK_RX_BUFFER_SIZE];
	struct rx_worker *worker = (struct rx_worker *)arg1;
	int ret;

	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	ret = isotp_bind(worker->ctx, can_dev, worker->rx_addr, worker->tx_addr, &fc_opts, K_FOREVER);
	if (ret != ISOTP_N_OK) {
		LOG_ERR("ISO-TP bind failed (broadcast=%d): %d", worker->is_broadcast, ret);
		return;
	}

	LOG_INF("ISO-TP RX ready id=0x%08x broadcast=%d", worker->rx_addr->ext_id,
		worker->is_broadcast);

	while (1) {
		int rx_len = isotp_recv(worker->ctx, rx_buffer, sizeof(rx_buffer),
					K_MSEC(CONFIG_CAN_LINK_RX_TIMEOUT_MS));

		if (rx_len == ISOTP_RECV_TIMEOUT) {
			continue;
		}

		if (rx_len < 0) {
			LOG_ERR("ISO-TP receive failed (broadcast=%d): %d", worker->is_broadcast, rx_len);
			continue;
		}

		if (rx_callback != NULL) {
			rx_callback(rx_buffer, (size_t)rx_len, extract_source_node(worker->ctx),
				    worker->is_broadcast, rx_callback_user_data);
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

	LOG_INF("CAN start board=%s node=%u peer=%u loopback=%d", CONFIG_BOARD,
		(unsigned int)CONFIG_CAN_LINK_NODE_ADDR, (unsigned int)CONFIG_CAN_LINK_PEER_NODE_ADDR,
		IS_ENABLED(CONFIG_CAN_LINK_LOOPBACK));
	LOG_INF("ISO-TP unicast tx=0x%08x rx_fc=0x%08x", isotp_ids.tx_addr.ext_id,
		isotp_ids.rx_fc_addr.ext_id);
	LOG_INF("ISO-TP broadcast tx=0x%08x rx=0x%08x", isotp_ids.broadcast_tx_addr.ext_id,
		isotp_ids.broadcast_bind_rx_addr.ext_id);

	rx_tid = k_thread_create(&unicast_rx_thread_data, unicast_rx_thread_stack,
				 K_THREAD_STACK_SIZEOF(unicast_rx_thread_stack), rx_thread,
				 &unicast_worker, NULL, NULL, CONFIG_CAN_LINK_RX_THREAD_PRIORITY, 0,
				 K_NO_WAIT);
	if (rx_tid == NULL) {
		LOG_ERR("Failed to create unicast RX thread");
		return -EIO;
	}
	k_thread_name_set(rx_tid, "can_rx_uni");

	rx_tid = k_thread_create(&broadcast_rx_thread_data, broadcast_rx_thread_stack,
				 K_THREAD_STACK_SIZEOF(broadcast_rx_thread_stack), rx_thread,
				 &broadcast_worker, NULL, NULL,
				 CONFIG_CAN_LINK_RX_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (rx_tid == NULL) {
		LOG_ERR("Failed to create broadcast RX thread");
		return -EIO;
	}
	k_thread_name_set(rx_tid, "can_rx_bc");

	is_initialized = true;

	return 0;
}

int can_link_send(const uint8_t *payload, size_t len)
{
	return can_link_send_to((uint8_t)CONFIG_CAN_LINK_PEER_NODE_ADDR, payload, len);
}

int can_link_send_to(uint8_t target_node, const uint8_t *payload, size_t len)
{
	int ret;
	struct isotp_msg_id tx_addr;
	struct isotp_msg_id rx_fc_addr;
	const uint8_t local = (uint8_t)CONFIG_CAN_LINK_NODE_ADDR;

	if (!is_initialized) {
		return -EACCES;
	}

	if ((payload == NULL) || (len == 0U)) {
		return -EINVAL;
	}
	if ((target_node == 0U) || (target_node == EPS_BROADCAST_NODE)) {
		return -EINVAL;
	}

	fill_isotp_fixed_id(&tx_addr, CONFIG_CAN_LINK_TELEMETRY_PRIORITY, EPS_FIXED_TATYPE_PHYSICAL,
			    target_node, local);
	fill_isotp_fixed_id(&rx_fc_addr, CONFIG_CAN_LINK_COMMAND_PRIORITY, EPS_FIXED_TATYPE_PHYSICAL,
			    local, target_node);

	k_mutex_lock(&send_lock, K_FOREVER);
	ret = isotp_send(&send_ctx, can_dev, payload, len, &tx_addr, &rx_fc_addr, NULL, NULL);
	k_mutex_unlock(&send_lock);

	return ret;
}

int can_link_send_broadcast(const uint8_t *payload, size_t len)
{
	int ret;

	if (!is_initialized) {
		return -EACCES;
	}

	if ((payload == NULL) || (len == 0U)) {
		return -EINVAL;
	}

	/* Classical CAN single-frame ISO-TP payload limit without ext addressing. */
	if (len > 7U) {
		return -EMSGSIZE;
	}

	k_mutex_lock(&send_lock, K_FOREVER);
	ret = isotp_send(&broadcast_send_ctx, can_dev, payload, len, &isotp_ids.broadcast_tx_addr,
			 &isotp_ids.rx_fc_addr, NULL, NULL);
	k_mutex_unlock(&send_lock);

	return ret;
}
