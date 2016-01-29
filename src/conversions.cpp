#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <libbladeRF.h>
#include "conversions.h"

int str2int(const char *str, int min, int max, bool *ok)
{
    long value;
    char *endptr;

    errno = 0;
    value = strtol(str, &endptr, 0);

    if (errno != 0 || value < (long)min || value > (long)max ||
        endptr == str || *endptr != '\0') {

        if (ok) {
            *ok = false;
        }

        return 0;
    }

    if (ok) {
        *ok = true;
    }
    return (int)value;
}

unsigned int str2uint(const char *str, unsigned int min, unsigned int max, bool *ok)
{
    unsigned long value;
    char *endptr;

    errno = 0;
    value = strtoul(str, &endptr, 0);

    if (errno != 0 ||
        value < (unsigned long)min || value > (unsigned long)max ||
        endptr == str || *endptr != '\0') {

        if (ok) {
            *ok = false;
        }

        return 0;
    }

    if (ok) {
        *ok = true;
    }
    return (unsigned int)value;
}


uint64_t str2uint64(const char *str, uint64_t min, uint64_t max, bool *ok)
{
    unsigned long long value;
    char *endptr;

    errno = 0;
    value = strtoull(str, &endptr, 0);

    if (errno != 0 || endptr == str || *endptr != '\0' ||
        value < (unsigned long long)min || value > (unsigned long long)max) {

        if (ok) {
            *ok = false;
        }

        return 0;
    }

    if (ok) {
        *ok = true;
    }

    return (uint64_t)value;
}

double str2double(const char *str, double min, double max, bool *ok)
{
    double value;
    char *endptr;

    errno = 0;
    value = strtod(str, &endptr);

    if (errno != 0 || value < min || value > max ||
        endptr == str || *endptr != '\0') {

        if (ok) {
            *ok = false;
        }

        return 0;
    }

    if (ok) {
        *ok = true;
    }

    return value;
}

/* MSVC workarounds */

#if _MSC_VER
    /* This appears to be missing <= 2012 */
#   ifndef INFINITY
#       define INFINITY (_HUGE * _HUGE)
#   endif

   /* As of MSVC 2013, INFINITY appears to be available, but
    * the compiler emits a warning for every usage, despite it
    * causing an overflow by design.
    *  https://msdn.microsoft.com/en-us/library/cwt7tyxx.aspx
    *
    * Oddly, we see warning 4056, despite math.h noting that 4756
    * will be induced (with the same description).
    */
#   pragma warning (push)
#   pragma warning (disable:4056)
#endif

double str2dbl_suffix(const char *str,
                      double min, double max,
                      const struct numeric_suffix suffixes[],
                      size_t num_suffixes, bool *ok)
{
    double value;
    char *endptr;
    size_t i;

    errno = 0;
    value = strtod(str, &endptr);

    /* If a number could not be parsed at the beginning of the string */
    if (errno != 0 || endptr == str) {
        if (ok) {
            *ok = false;
        }

        return 0;
    }

    /* Loop through each available suffix */
    for (i = 0; i < num_suffixes; i++) {
        /* If the suffix appears at the end of the number */
        if (!strcasecmp(endptr, suffixes[i].suffix)) {
            /* Apply the multiplier */
            value *= suffixes[i].multiplier;
            break;
        }
    }

    /* Test for overflow */
    if (value == INFINITY || value == -INFINITY) {
        if (ok) {
            *ok = false;
        }

        return 0;
    }

    /* Check that the resulting value is in bounds */
    if (value > max || value < min) {
        if (ok) {
            *ok = false;
        }

        return 0;
    }

    if (ok) {
        *ok = true;
    }

    return value;
}
#if _MSC_VER
#   pragma warning(pop)
#endif

unsigned int str2uint_suffix(const char *str,
                             unsigned int min, unsigned int max,
                             const struct numeric_suffix suffixes[],
                             size_t num_suffixes, bool *ok)
{
    return (unsigned int) str2dbl_suffix(str, min, max,
                                         suffixes, num_suffixes, ok);
}

uint64_t str2uint64_suffix(const char *str,
                           uint64_t min, uint64_t max,
                           const struct numeric_suffix suffixes[],
                           size_t num_suffixes, bool *ok)
{
    /* FIXME: Potential loss of precision on min/max here */
    return (uint64_t) str2dbl_suffix(str, (double) min, (double) max,
                                     suffixes, num_suffixes, ok);
}

int str2lnagain(const char *str, bladerf_lna_gain *gain)
{
    *gain = BLADERF_LNA_GAIN_MAX;

    if (!strcasecmp("max", str) ||
        !strcasecmp("BLADERF_LNA_GAIN_MAX", str)) {
        *gain = BLADERF_LNA_GAIN_MAX;
        return 0;
    } else if (!strcasecmp("mid", str) ||
               !strcasecmp("BLADERF_LNA_GAIN_MID", str)) {
        *gain = BLADERF_LNA_GAIN_MID;
        return 0;
    } else if (!strcasecmp("bypass", str) ||
               !strcasecmp("BLADERF_LNA_GAIN_BYPASS", str)) {
        *gain = BLADERF_LNA_GAIN_BYPASS;
        return 0;
    } else {
        *gain = BLADERF_LNA_GAIN_UNKNOWN;
        return -1;
    }
}
