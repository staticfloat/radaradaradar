#include <libbladeRF.h>

struct opts_struct {
    // Our verbosity level
    int verbosity;

    // The frequency we will tune to
    unsigned int freq;

    // The samplerate/bandwidth at which we will be capturing data
    unsigned int samplerate;

    // Number of milliseconds to run for
    unsigned int exit_timer;

    // Whether our receive and transmit lowpass filters are enabled.
    bool rx_lpf_enabled, tx_lpf_enabled;

    // Gains of amplifiers within the bladeRF
    bladerf_lna_gain lna;
    char rxvga1, rxvga2;
    char txvga1, txvga2;

    // Internal buffer settings (we don't have a way to set these right now)
    unsigned int num_buffers;
    unsigned int buffer_size;
    unsigned int num_transfers;
    unsigned int timeout_ms;

    // bladeRF device name
    char * devstr;
};
extern struct opts_struct opts;

void parse_options(int argc, char ** argv);
void cleanup_options();
