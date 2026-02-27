#ifndef CAN_LINK_H_
#define CAN_LINK_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*can_link_rx_handler_t)(const uint8_t *payload, size_t len, void *user_data);

int can_link_init(can_link_rx_handler_t rx_handler, void *user_data);
int can_link_send(const uint8_t *payload, size_t len);
uint32_t can_link_node_id(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_LINK_H_ */
