#ifndef GLOBALS_H
#define GLOBALS_H

#include "board.h"
#include "hashtable.h"
#include "threads.h"
#include "search.h"

S_BOARD global_board;
S_HASHTABLE global_tp_table;
S_THREADS global_thread_table;
S_SEARCH_SETTINGS global_search_settings;

#endif /* GLOBALS_H */
