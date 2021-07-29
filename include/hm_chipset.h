#ifndef HM_CHIPSET_H
#define HM_CHIPSET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hm_chipset {
    char *name;

    /**
     * @brief User default init.
     * 
     * @return int
     * @retval  HM_SUCCESS              Chipset init success.
     * @retval  HM_CHIPSET_INIT_ERROR   Chipset init error.
    */
    int (*init)(void);
    
} hm_chipset_t;

/**
 * @brief Get hci middleware chipset instance.
 * 
 * @return hm_chipset_t* 
 * @retval  Non-NULL    Chipset instance.
 * @retval  NULL        Error.
 */
extern hm_chipset_t* hm_chipset_get_instance(void);

/**
 * @brief Send a hci command, used in chipset module.
 * 
 * @param buf       Buffer to store a hci command.
 * @param size      HCI command size with bytes.
 * 
 * @return int 
 * @retval  HM_SUCCESS           Send success.
 * @retval  -HM_NO_MEMORY        Command memory pool is not enough.
 * @retval  -HM_UART_SEND_ERR    Uart send command error.
 */
extern int chip_hci_cmd_send(uint8_t *buf, uint16_t size);

/**
 * @brief Read a hci event, used in chipset module. 
 *      This function will block until a hci event received or wait time timeout. 
 *      Usually used in chipset init.
 * 
 * @param buf       Buffer to store a hci event.
 * @param size      Buffer size.
 * @param ms        Waitting time in ms. Specially, 
 *              RT_WAITING_NO means no wait, 
 *              RT_WAITING_FOREVER means wait forever.
 * 
 * @return int 
 * @retval  HM_SUCCESS      Read hci event success.
 * @retval  -HM_TIMEOUT     Timeout.
 */
extern int chip_hci_event_read(uint8_t *buf, uint16_t size, int ms);

/**
 * @brief This function will continually send HCI-Reset-Command until 
 *      remote chipset respond a HCI-Somplete-Command-Event.
 * 
 * @note Because a chipset may not be ready, if we only send a reset command, 
 *      remote chipset may not respond any event. So we should continually 
 *      send reset command in chipset init start and end.
 */
extern void chip_send_hci_reset_cmd_until_ack(void);

#ifdef __cplusplus
}
#endif

#endif /* HM_CHIPSET_H */
