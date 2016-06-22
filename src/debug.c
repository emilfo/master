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
    return (pce >= 0 && pce < 15);
}

int valid_bool(const int val)
{
    return (val == 0 || val == 1);
}
