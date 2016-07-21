#  include <sys/time.h>

#include <stdlib.h>

unsigned long cur_time_millis(void) {
    struct timeval timeval;
    gettimeofday(&timeval, NULL);
    return (long) (long)timeval.tv_sec * 1000 + (timeval.tv_usec / 1000);
}
