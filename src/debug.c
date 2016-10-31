#include <stdio.h>
#include "globals.h"

int valid_sq(const int sq) 
{
    return (sq >= 0 && sq < 64);
}

int valid_side(const int side) 
{
    return (side == WHITE || side == BLACK);
}

int valid_piece(const int pce) 
{
    return (pce >= 1 && pce <= 12);
}

int valid_piece_or_empty(const int pce) 
{
    return (pce >= 0 && pce <= 12);
}

int valid_bool(const int val)
{
    return (val == 0 || val == 1);
}

int print_depth(int thread_id, int cur_depth)
{
    if (thread_id == 0)
        printf("%2d|  |  |  |  |  |  |  \n",cur_depth);
    if (thread_id == 1)
        printf("  |%2d|  |  |  |  |  |  \n",cur_depth);
    if (thread_id == 2)
        printf("  |  |%2d|  |  |  |  |  \n",cur_depth);
    if (thread_id == 3)
        printf("  |  |  |%2d|  |  |  |  \n",cur_depth);
    if (thread_id == 4)
        printf("  |  |  |  |%2d|  |  |  \n",cur_depth);
    if (thread_id == 5)
        printf("  |  |  |  |  |%2d|  |  \n",cur_depth);
    if (thread_id == 6)
        printf("  |  |  |  |  |  |%2d|  \n",cur_depth);
    if (thread_id == 7)
        printf("  |  |  |  |  |  |  |%2d\n",cur_depth);

    return 1;
}

