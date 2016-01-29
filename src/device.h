#include <stdbool.h>
#include <stdint.h>
#include <queue>
#include <time.h>
#include <pthread.h>

struct device_data_struct {
    // Our bladeRF context object
    struct bladerf *dev;

    // The timestamp at which we should try to transmit our next buffer
    uint64_t next_tx_time;
};
extern struct device_data_struct device_data;

bool open_device(void);
void close_device(void);
