#include <stdint.h>

int str2int(const char *str, int min, int max, bool *ok);
unsigned int str2uint(const char *str, unsigned int min, unsigned int max, bool *ok);
uint64_t str2uint64(const char *str, uint64_t min, uint64_t max, bool *ok);
double str2double(const char *str, double min, double max, bool *ok);
int str2lnagain(const char *str, bladerf_lna_gain *gain);

double str2dbl_suffix(const char *str,
                      double min, double max,
                      const struct numeric_suffix suffixes[],
                      size_t num_suffixes, bool *ok);

unsigned int str2uint_suffix(const char *str,
                             unsigned int min, unsigned int max,
                             const struct numeric_suffix suffixes[],
                             size_t num_suffixes, bool *ok);

uint64_t str2uint64_suffix(const char *str,
                           uint64_t min, uint64_t max,
                           const struct numeric_suffix suffixes[],
                           size_t num_suffixes, bool *ok);

typedef struct numeric_suffix {
   const char *suffix;
   int multiplier;
} numeric_suffix;
