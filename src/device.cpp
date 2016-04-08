#include <libbladeRF.h>
#include "device.h"
#include "options.h"
#include "util.h"
#include <stdlib.h>

struct device_data_struct device_data;

bool open_device(void)
{
    int status;

    // Initialize everything in device_data to zero
    memset(&device_data, 0, sizeof(struct device_data_struct));

    LOG("Opening and initializing device...\n");
    status = bladerf_open(&device_data.dev, opts.devstr);
    if( status != 0 ) {
        ERROR("Failed to open device: %s\n", bladerf_strerror(status));
        goto out;
    }

    status = bladerf_set_frequency(device_data.dev, BLADERF_MODULE_RX, opts.freq);
    if( status != 0 ) {
        ERROR("Failed to set RX frequency %u: %s\n", opts.freq, bladerf_strerror(status));
        goto out;
    } else {
        char str[9];
        double2str_suffix(str, opts.freq, freq_suffixes, NUM_FREQ_SUFFIXES);
        INFO("  RX frequency: %sHz\n", str);
    }

    status = bladerf_set_frequency(device_data.dev, BLADERF_MODULE_TX, opts.freq);
    if( status != 0 ) {
        ERROR("Failed to set TX frequency %u: %s\n", opts.freq, bladerf_strerror(status));
        goto out;
    } else {
        char str[9];
        double2str_suffix(str, opts.freq, freq_suffixes, NUM_FREQ_SUFFIXES);
        INFO("  TX frequency: %sHz\n", str);
    }

    status = bladerf_set_sample_rate(device_data.dev, BLADERF_MODULE_RX, opts.samplerate, NULL);
    if( status != 0 ) {
        ERROR("Failed to set RX sample rate: %s\n", bladerf_strerror(status));
        goto out;
    } else {
        char str[9];
        double2str_suffix(str, opts.samplerate, freq_suffixes, NUM_FREQ_SUFFIXES);
        INFO("  RX samplerate: %ssps\n", str);
    }

    status = bladerf_set_sample_rate(device_data.dev, BLADERF_MODULE_TX, opts.samplerate, NULL);
    if( status != 0 ) {
        ERROR("Failed to set TX sample rate: %s\n", bladerf_strerror(status));
        goto out;
    } else {
        char str[9];
        double2str_suffix(str, opts.samplerate, freq_suffixes, NUM_FREQ_SUFFIXES);
        INFO("  TX samplerate: %ssps\n", str);
    }

    if( !opts.rx_lpf_enabled ) {
        status = bladerf_set_lpf_mode(device_data.dev, BLADERF_MODULE_RX, BLADERF_LPF_BYPASSED);
        if( status != 0 ) {
            ERROR("Failed to bypass RX low pass filter: %s\n", bladerf_strerror(status));
            goto out;
        } else {
            INFO("  RX LPF: Bypassed\n");
        }
    }

    if( !opts.tx_lpf_enabled ) {
        status = bladerf_set_lpf_mode(device_data.dev, BLADERF_MODULE_TX, BLADERF_LPF_BYPASSED);
        if( status != 0 ) {
            ERROR("Failed to bypass TX low pass filter: %s\n", bladerf_strerror(status));
            goto out;
        } else {
            INFO("  TX LPF: Bypassed\n");
        }
    }

    status = bladerf_set_lna_gain(device_data.dev, opts.lna);
    if( status != 0 ) {
        bool ok;
        ERROR("Failed to set LNA gain to %ddB: %s\n", bladerf_lna_gain_to_db(opts.lna, &ok), bladerf_strerror(status));
        goto out;
    } else {
        bool ok;
        INFO("  LNA Gain: %ddB\n", bladerf_lna_gain_to_db(opts.lna, &ok));
    }

    status = bladerf_set_rxvga1(device_data.dev, opts.rxvga1);
    if( status != 0 ) {
        ERROR("Failed to set RX VGA1 gain: %s\n", bladerf_strerror(status));
        goto out;
    } else {
        INFO("  RX VGA1 gain: %ddB\n", opts.rxvga1);
    }

    status = bladerf_set_rxvga2(device_data.dev, opts.rxvga2);
    if( status != 0 ) {
        ERROR("Failed to set RX VGA2 gain: %s\n", bladerf_strerror(status));
        goto out;
    } else {
        INFO("  RX VGA2 gain: %ddB\n", opts.rxvga2);
    }

    status = bladerf_set_txvga1(device_data.dev, opts.txvga1);
    if( status != 0 ) {
        ERROR("Failed to set TX VGA1 gain: %s\n", bladerf_strerror(status));
        goto out;
    } else {
        INFO("  TX VGA1 gain: %ddB\n", opts.txvga1);
    }

    status = bladerf_set_txvga2(device_data.dev, opts.txvga2);
    if( status != 0 ) {
        ERROR("Failed to set TX VGA2 gain: %s\n", bladerf_strerror(status));
        goto out;
    } else {
        INFO("  TX VGA2 gain: %ddB\n", opts.txvga2);
    }

    status = bladerf_sync_config(device_data.dev, BLADERF_MODULE_RX,
                                 BLADERF_FORMAT_SC16_Q11_META, opts.num_buffers,
                                 opts.buffer_size, opts.num_transfers, opts.timeout_ms);
    if( status != 0 ) {
        ERROR("Failed to sync RX config: %s\n", bladerf_strerror(status));
        goto out;
    }

    status = bladerf_sync_config(device_data.dev, BLADERF_MODULE_TX,
                                 BLADERF_FORMAT_SC16_Q11_META, opts.num_buffers,
                                 opts.buffer_size, opts.num_transfers, opts.timeout_ms);
    if( status != 0 ) {
        ERROR("Failed to sync TX config: %s\n", bladerf_strerror(status));
        goto out;
    }

    status = bladerf_enable_module(device_data.dev, BLADERF_MODULE_RX, true);
    if( status != 0 ) {
        ERROR("Failed to enable RX module: %s\n", bladerf_strerror(status));
        goto out;
    }

    status = bladerf_enable_module(device_data.dev, BLADERF_MODULE_TX, true);
    if( status != 0 ) {
        ERROR("Failed to enable TX module: %s\n", bladerf_strerror(status));
        goto out;
    }

    // Get our next transmission time
    status = bladerf_get_timestamp(device_data.dev, BLADERF_MODULE_TX, &device_data.next_tx_time);
    if (status != 0) {
        ERROR("Failed to get TX timestamp: %s\n", bladerf_strerror(status));
        goto out;
    }

out:
    if (status != 0) {
        bladerf_close(device_data.dev);
        return false;
    }
    return true;
}

void close_device(void)
{
    int status = 0;
    LOG("\nClosing device...");

    /* Disable RX module, shutting down our underlying RX stream */
    status = bladerf_enable_module(device_data.dev, BLADERF_MODULE_RX, false);
    if (status != 0) {
        ERROR("Failed to disable RX module: %s\n", bladerf_strerror(status));
    }

    status = bladerf_enable_module(device_data.dev, BLADERF_MODULE_TX, false);
    if (status != 0) {
        ERROR("Failed to disable TX module: %s\n", bladerf_strerror(status));
    }

    // Deinitialize and free resources
    bladerf_close(device_data.dev);
    LOG(".Done!\n");
}
