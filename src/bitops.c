#include "bitops.h"
#include "board.h"
/** 
 * Returns a count of all bits set to 1 
 * M1 puts count of each 2 bit into those 2 bits
 * ie. 0b1101 becomes 1001, which is 10+01, or 2+1 = 3
 * M2 then puts the count of 4 bits into those 4 bits
 * ie. 1001 (from the previous operation) then becomes 0011 = 3 
 * and so on. Taken from MIT HAKMEM (AI Memo 239) 
 */
int bit_count(u64 bitmap)
{
    static const u64 M1  = 0x5555555555555555;//0101010101...
    static const u64 M2  = 0x3333333333333333;//0011001100...
    static const u64 M4  = 0x0f0f0f0f0f0f0f0f;//0000111100...
    static const u64 M8  = 0x00ff00ff00ff00ff;//0000000011...
    static const u64 M16 = 0x0000ffff0000ffff;//and so on
    static const u64 M32 = 0x00000000ffffffff;

    bitmap = (bitmap & M1)  + ((bitmap >> 1) & M1);
    bitmap = (bitmap & M2)  + ((bitmap >> 1) & M2);
    bitmap = (bitmap & M4)  + ((bitmap >> 1) & M4);
    bitmap = (bitmap & M8)  + ((bitmap >> 1) & M8);
    bitmap = (bitmap & M16) + ((bitmap >> 1) & M16);
    bitmap = (bitmap & M32) + ((bitmap >> 1) & M32);

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

