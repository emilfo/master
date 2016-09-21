#ifndef GLOBALS_H
#define GLOBALS_H

#include "board.h"
#include "hashtable.h"
#include "threads.h"
#include "search.h"

extern S_BOARD g_board;
extern S_HASHTABLE g_hash_table;
extern S_THREADS g_thread_table;
extern S_SEARCH_SETTINGS g_search_info;

extern volatile int g_depth;
extern volatile int g_currently_searching;

#endif /* GLOBALS_H */
