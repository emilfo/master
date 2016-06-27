#ifdef WIN32
#  include <time.h>
#else
#  include <sys/time.h>
#endif

#include <stdlib.h>

unsigned int cur_time_millis(void) {
#ifdef WIN32
    return (unsigned int) GetTickCount();
#else
    struct timeval timeval;
    gettimeofday(&timeval, NULL);
    return timeval.tv_sec * 1000 + (timeval.tv_usec / 1000);
#endif
}
