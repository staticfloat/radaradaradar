// Include libbladeRF conversion code
#include "options.h"
#include "util.h"
#include "conversions.h"
#include <libbladeRF.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <limits.h>

// Instantiate our options struct
struct opts_struct opts;

void version()
{
    printf("RadarAdarAdar v0.0.1\n");
}

void usage()
{
    version();
    printf("Usage:\n");
    printf("  radar [options]\n");
    printf("  radar (-h | --help)\n");
    printf("  radar --version\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h --help                  Show this screen.\n");
    printf("  -V --version               Show version.\n");
    printf("  -v --verbose               Raise verbosity level (can be repeated)\n");
    printf("  -f --frequency=<freq>      Frequency to transmit on [default: 1G]\n");
    printf("  -e --exit-timer=<et>       Set capture time (examples: \"5h\", \"30m\") [default: 0]\n");
    printf("  -b --bandwidth=<bw>        Capture bandwidth [default: BLADERF_BANDWIDTH_MAX].\n");
    printf("  -g --lna-gain=<g>          Set LNA gain either as numeric dB value (0, %d, %d) or\n", BLADERF_LNA_GAIN_MID_DB, BLADERF_LNA_GAIN_MAX_DB);
    printf("                             as symbolic (bypass, min, max). [default: max]\n");
    printf("  -o --rx-vga1=<g>           Set rxvga1 gain either as numeric dB value [%d...%d] or\n", BLADERF_RXVGA1_GAIN_MIN, BLADERF_RXVGA1_GAIN_MAX);
    printf("                             as symbolic (min, max).  [default: min]\n");
    printf("  -w --rx-vga2=<g>           Set rxvga2 gain either as numeric dB value [%d...%d] or\n", BLADERF_RXVGA2_GAIN_MIN, BLADERF_RXVGA2_GAIN_MAX);
    printf("                             as symbolic (min, max).  [default: min]\n");
    printf("  -q --tx-vga1=<g>           Set txvga1 gain either as numeric dB value [%d...%d] or\n", BLADERF_TXVGA1_GAIN_MIN, BLADERF_TXVGA1_GAIN_MAX);
    printf("                             as symbolic (min, max).  [default: min]\n");
    printf("  -r --tx-vga2=<g>           Set txvga2 gain either as numeric dB value [%d...%d] or\n", BLADERF_TXVGA2_GAIN_MIN, BLADERF_TXVGA2_GAIN_MAX);
    printf("                             as symbolic (min, max).  [default: min]\n");
    printf("  -R --rx-lpf                Enable RX LPF [default: disabled]\n");
    printf("  -T --tx-lpf                Enable TX LPF [default: disabled]\n");
    printf("  -d --device=<d>            Device identifier [default: ]\n");
}

static const struct option longopts[] = {
    { "help",               no_argument,        0, 'h' },
    { "version",            no_argument,        0, 'V' },
    { "verbose",            no_argument,        0, 'v' },
    { "frequency",          required_argument,  0, 'f' },
    { "exit-timer",         required_argument,  0, 'e' },
    { "bandwidth",          required_argument,  0, 'b' },
    { "lna-gain",           required_argument,  0, 'g' },
    { "rxvga1-gain",        required_argument,  0, 'o' },
    { "rxvga2-gain",        required_argument,  0, 'w' },
    { "txvga1-gain",        required_argument,  0, 'q' },
    { "txvga2-gain",        required_argument,  0, 'r' },
    { "rx-lpf",             no_argument,        0, 'R' },
    { "tx-lpf",             no_argument,        0, 'T' },
    { "device",             required_argument,  0, 'd' },
    { 0,                    0,                  0,  0  },
};


// Macro to set default values that are initialized to zero
#define DEFAULT(field, val) if( field == 0 ) { field = val; }
#define OPTSTR "hvVRTe:f:b:g:o:w:q:r:d:"

void parse_options(int argc, char ** argv)
{
    // First thing we do is initialize the entire opts struct to zero
    memset(&opts, sizeof(opts), 0);

    // Declare some temporary variables
    bool ok;

    // Next, parse options into opts:
    int optidx;
    int c = getopt_long(argc, argv, OPTSTR, longopts, &optidx);
    do {
        switch(c) {
            case 'h':
                usage();
                exit(0);
                break;
            case 'v':
                opts.verbosity++;
                break;
            case 'V':
                version();
                exit(0);
                break;
            case 'f':
                opts.freq = str2uint_suffix(optarg, BLADERF_FREQUENCY_MIN,
                                            BLADERF_FREQUENCY_MAX, freq_suffixes,
                                            NUM_FREQ_SUFFIXES, &ok);
                if( !ok ) {
                    ERROR("Invalid frequency \"%s\"\n", optarg);
                    ERROR("Valid values given in Hertz (ex: \"1000000000\")\n");
                    ERROR("or, equivalently, with units (ex: \"1.1G\" or \"290M\")\n");
                    exit(1);
                }
                break;
            case 'e':
                opts.exit_timer = str2uint_suffix(optarg, 0, UINT_MAX, time_suffixes,
                                                  NUM_TIME_SUFFIXES, &ok);
                if( !ok ) {
                    ERROR("Invalid exit timer \"%s\"\n", optarg);
                    ERROR("Valid values given in milliseconds (ex: \"60000\")\n");
                    ERROR("or, equivalently, with units: (ex: \"60s\" or \"1m\")\n");
                    exit(1);
                }
                break;
            case 'b':
                opts.samplerate = str2uint_suffix(optarg, BLADERF_BANDWIDTH_MIN,
                                                  BLADERF_BANDWIDTH_MAX, freq_suffixes,
                                                  NUM_FREQ_SUFFIXES, &ok);
                if( !ok ) {
                    ERROR("Invalid samplerate \"%s\"\n", optarg);
                    ERROR("Valid range: [%u, %u]\n", BLADERF_BANDWIDTH_MIN, BLADERF_BANDWIDTH_MAX);
                    exit(1);
                }
                break;
            case 'g': {
                // Try to interpret it as an integer
                unsigned char db = str2uint(optarg, 0, BLADERF_LNA_GAIN_MAX_DB, &ok);
                if( ok ) {
                    opts.lna = bladerf_db_to_lna_gain(db, &ok);
                    if( !ok ) {
                        ERROR("Invalid numeric LNA gain \"%s\"\n", optarg);
                        ERROR("Valid values: [0, %d, %d]\n", BLADERF_LNA_GAIN_MID_DB, BLADERF_LNA_GAIN_MAX_DB);
                        exit(1);
                    }
                } else {
                    // If that fails, try interpreting it as a symbol:
                    if( str2lnagain(optarg, &opts.lna) == -1 ) {
                        ERROR("Unable to parse symbolic LNA gain \"%s\"\n", optarg);
                        ERROR("Valid values: [\"bypass\", \"mid\", \"max\"]\n");
                        exit(1);
                    }
                }
            }   break;
            case 'o':
                opts.rxvga1 = str2int(optarg, BLADERF_RXVGA1_GAIN_MIN,
                                    BLADERF_RXVGA1_GAIN_MAX, &ok);
                if( !ok ) {
                    ERROR("Invalid RXVGA1 gain \"%s\"\n", optarg);
                    ERROR("Valid range: [%u, %u]\n", BLADERF_RXVGA1_GAIN_MIN, BLADERF_RXVGA1_GAIN_MAX);
                    exit(1);
                }
                break;
            case 'w':
                opts.rxvga2 = str2int(optarg, BLADERF_RXVGA2_GAIN_MIN,
                                    BLADERF_RXVGA2_GAIN_MAX, &ok);
                if( !ok ) {
                    ERROR("Invalid RXVGA2 gain \"%s\"\n", optarg);
                    ERROR("Valid range: [%u, %u]\n", BLADERF_RXVGA2_GAIN_MIN, BLADERF_RXVGA2_GAIN_MAX);
                    exit(1);
                }
                break;
            case 'q':
                opts.txvga1 = str2int(optarg, BLADERF_TXVGA1_GAIN_MIN,
                                    BLADERF_TXVGA1_GAIN_MAX, &ok);
                if( !ok ) {
                    ERROR("Invalid TXVGA1 gain \"%s\"\n", optarg);
                    ERROR("Valid range: [%d, %d]\n", BLADERF_TXVGA1_GAIN_MIN, BLADERF_TXVGA1_GAIN_MAX);
                    exit(1);
                }
                break;
            case 'r':
                opts.txvga2 = str2int(optarg, BLADERF_TXVGA2_GAIN_MIN,
                                    BLADERF_TXVGA2_GAIN_MAX, &ok);
                if( !ok ) {
                    ERROR("Invalid TXVGA2 gain \"%s\"\n", optarg);
                    ERROR("Valid range: [%d, %d]\n", BLADERF_TXVGA2_GAIN_MIN, BLADERF_TXVGA2_GAIN_MAX);
                    exit(1);
                }
                break;
            case 'R':
                opts.rx_lpf_enabled = true;
                break;
            case 'T':
                opts.tx_lpf_enabled = true;
                break;
            case 'd':
                opts.devstr = strdup(optarg);
                break;
        }

        c = getopt_long(argc, argv, OPTSTR, longopts, &optidx);
    } while (c != -1);

    // Set defaults for everything that didn't get an explicit value:
    DEFAULT(opts.verbosity, 0);
    DEFAULT(opts.freq, 2000000000);
    DEFAULT(opts.samplerate, BLADERF_BANDWIDTH_MAX);
    DEFAULT(opts.lna, BLADERF_LNA_GAIN_MAX);
    DEFAULT(opts.rxvga1, BLADERF_RXVGA1_GAIN_MIN);
    DEFAULT(opts.rxvga2, BLADERF_RXVGA2_GAIN_MIN);
    DEFAULT(opts.txvga1, BLADERF_TXVGA1_GAIN_MIN);
    DEFAULT(opts.txvga2, BLADERF_TXVGA2_GAIN_MIN);
    DEFAULT(opts.devstr, strdup(""));
    DEFAULT(opts.num_buffers, 32);
    DEFAULT(opts.buffer_size, 8192);
    DEFAULT(opts.num_transfers, 8);
    DEFAULT(opts.timeout_ms, 1000);

    // Do we have excess arguments?
    if( argc - optind > 0 ) {
        ERROR("Unknown extra arguments, ignoring:\n");
        for( int idx = optind + 1; idx < argc; ++idx )
            ERROR("%s\n", argv[idx]);
    }
}

void cleanup_options(void)
{
    free(opts.devstr);
}
