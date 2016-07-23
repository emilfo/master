#include <stdio.h>

#include "bitops.h"
#include "defs.h"

/** 
 * Returns a count of all bits set to 1 
 * M1 puts count of each 2 bit into those 2 bits
 * ie. 0b1101 becomes 1001, which is 10+01, or 2+1 = 3
 * M2 then puts the count of 4 bits into those 4 bits
 * ie. 1001 (from the previous operation) then becomes 0011 = 3 
 * and so on. taken from:
 * https://chessprogramming.wikispaces.com/Population+Count
 * found on Donald Knuths The art of computer programming (2009)
 */
int bit_count(u64 bitmap)
{
    static const u64 K1  = 0x5555555555555555;//0101010101...
    static const u64 K2  = 0x3333333333333333;//0011001100...
    static const u64 K4  = 0x0f0f0f0f0f0f0f0f;//0000111100...
    static const u64 Kf  = 0x0101010101010101;

    bitmap =  bitmap        - ((bitmap >> 1) & K1);
    bitmap = (bitmap & K2)  + ((bitmap >> 2) & K2);
    bitmap = (bitmap & K4)  + ((bitmap >> 4) & K4);
    bitmap = (bitmap * Kf)  >> 56;

    return (int) bitmap;
}

/* returns index of the first bit in the bitmap 
 * uses De Bruijn multiplication
 * http://chessprogramming.wikispaces.com/BitScan
 *
 * bitScanForward
 * @author Kim Walisch (2012)
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
int lsb1_index(u64 bitmap)
{
    assert(bitmap != 0); //Can't be called with bitmap=0

    const int index64[64] = {
        0, 47,  1, 56, 48, 27,  2, 60,
        57, 49, 41, 37, 28, 16,  3, 61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11,  4, 62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30,  9, 24,
        13, 18,  8, 12,  7,  6,  5, 63
    };

    const u64 debruijn64 = 0x03f79d71b4cb0a89;


    return index64[((bitmap ^ (bitmap-1)) * debruijn64) >> 58];
}

/**
 * returns index of last bit in the bitmap, slower than lsb
 * @Author Eugene Nalimov
 */
int msb1_index(u64 bitmap) 
{
    assert(bitmap != 0); //Can't be called with bitmap=0

    int result = 0;

    if (bitmap > 0xFFFFFFFF){
        bitmap >>= 32;
        result = 32;
    }
    if (bitmap > 0xFFFF){
        bitmap >>= 16;
        result += 16;
    }
    if (bitmap > 0xFF){
        bitmap >>= 8;
        result += 8;
    }

    return result + MS1BTABLE[bitmap];
}

/**
 * Taken from chessprogramming wiki:
 * https://chessprogramming.wikispaces.com/Flipping+Mirroring+and+Rotating#The%20whole%20Bitboard
 * Flip a bitboard vertically about the centre ranks.
 * Rank 1 is mapped to rank 8 and vice versa.
 * @param x any bitboard
 * @return bitboard x flipped vertically
 *      */
u64 flipVertical(u64 x) {
    return  (x << 56) |
        ((x << 40) & (u64)0x00ff000000000000) |
        ((x << 24) & (u64)0x0000ff0000000000) |
        ((x <<  8) & (u64)0x000000ff00000000) |
        ((x >>  8) & (u64)0x00000000ff000000) |
        ((x >> 24) & (u64)0x0000000000ff0000) |
        ((x >> 40) & (u64)0x000000000000ff00) |
        (x >> 56);
}
