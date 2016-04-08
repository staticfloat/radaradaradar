#include <stdio.h>
#include <libbladeRF.h>
#include "util.h"
#include "device.h"
#include "options.h"
#include <math.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>


bool keep_running = true;
struct sigaction old_sigint_action;

void sigint_handler(int dummy)
{
    LOG("\nGracefully shutting down...");
    keep_running = false;
    sigaction(SIGINT, &old_sigint_action, NULL);
}

void wait_preciousssss() {
    int status = 0;

    // Give ourselves 5ms time to setup the next transmission
    uint64_t wait_ts = device_data.next_tx_time - 5*opts.samplerate/1000;
    uint64_t curr_ts = 0;

    status = bladerf_get_timestamp(device_data.dev, BLADERF_MODULE_TX, &curr_ts);
    if( status != 0 ) {
        ERROR("Failed to get timestamp: %s\n", bladerf_strerror(status));
        return;
    }

    while( curr_ts < wait_ts ) {
        usleep(1000);

        status = bladerf_get_timestamp(device_data.dev, BLADERF_MODULE_TX, &curr_ts);
        if( status != 0 ) {
            ERROR("Failed to get timestamp: %s\n", bladerf_strerror(status));
            return;
        }
    }
}

// We will transmit barker 11 codes lasting time_ms in one burst here
void transmit_barker11(unsigned int time_ms) {
    int status = 0;
    struct bladerf_metadata meta;

    memset(&meta, 0, sizeof(meta));

    /* Send entire burst worth of samples in one function call */
    meta.flags = BLADERF_META_FLAG_TX_BURST_START |
                 BLADERF_META_FLAG_TX_NOW |
                 BLADERF_META_FLAG_TX_BURST_END;

    // Try to schedule this transmission for the next transmission time
    //meta.timestamp = device_data.next_tx_time;

    unsigned int N = (time_ms*opts.samplerate)/(1000*11);
    int16_t * buff = (int16_t *)malloc(sizeof(int16_t)*2*N*11);

    // Slam out barker11 datapoints with some gain into the real part of buff
    for( unsigned int i=0; i<N; ++i ) {
        buff[22*i +  0] = 2047;
        buff[22*i +  2] = 2047;
        buff[22*i +  4] = 2047;
        buff[22*i +  6] = -2047;
        buff[22*i +  8] = -2047;
        buff[22*i + 10] = -2047;
        buff[22*i + 12] = 2047;
        buff[22*i + 14] = -2047;
        buff[22*i + 16] = -2047;
        buff[22*i + 18] = 2047;
        buff[22*i + 20] = -2047;
    }

    // Hand these samples off to libbladeRF
    status = bladerf_sync_tx(device_data.dev, buff, N*11, &meta, opts.timeout_ms);
    if( status != 0 ) {
        ERROR("TX failed for %d samples: %s\n", N*11, bladerf_strerror(status));
    }

    // Update next_transmission_time, bumping next_tx_time forward if we have
    // fallen behind somehow
    uint64_t curr_ts = 0;
    status = bladerf_get_timestamp(device_data.dev, BLADERF_MODULE_TX, &curr_ts);
    if( status != 0 ) {
        ERROR("Could not get timestamp: %s\n", bladerf_strerror(status));
    } else {
        if( device_data.next_tx_time < curr_ts )
            device_data.next_tx_time = curr_ts;
    }
    device_data.next_tx_time += N*11;
}

int main(int argc, char ** argv)
{
    parse_options(argc, argv);

    if( opts.verbosity > 2 )
        bladerf_log_set_verbosity(BLADERF_LOG_LEVEL_DEBUG);

    // Open our bladeRF
    if( !open_device() )
        return 1;

    // Setup SIGINT handler so we can gracefully quit
    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigaction(SIGINT, &act, &old_sigint_action);

    // Keep track of the time
    timeval tv_start, tv_status, tv;
    gettimeofday(&tv_start, NULL);
    tv_status = tv_start;

    // Begin transmission loop
    while( keep_running ) {
        // Always get current time
        gettimeofday(&tv, NULL);

        // Check to see if we've overstayed our welcome
        if( opts.exit_timer != 0 && msdiff(tv, tv_start) > opts.exit_timer ) {
            keep_running = false;
            break;
        }

        // Otherwise, transmit!
        printf(".");
        fflush(stdout);
        transmit_barker11(10);
        wait_preciousssss();
    }

    // Stop worker threads
    close_device();
    cleanup_options();
    LOG("Shutdown complete!\n")
    return 0;
}
