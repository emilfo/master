#include <stdio.h>
#include "globals.h"
#include "data.h"

const int files[64] = {
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
};

const int ranks[64] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8,
};

const u64 _DIAGA8H1MAGICS[15] = {
    0x0,
    0x0,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0080808080808080,
    0x0040404040404040,
    0x0020202020202020,
    0x0010101010101010,
    0x0008080808080808,
    0x0,
    0x0
};
const u64 _DIAGA1H8MAGICS[15] = {
    0x0,
    0x0,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x8080808080808000,
    0x4040404040400000,
    0x2020202020000000,
    0x1010101000000000,
    0x0808080000000000,
    0x0,
    0x0
};

const u64 _FILEMAGICS[8] = {
    0x8040201008040200,
    0x4020100804020100,
    0x2010080402010080,
    0x1008040201008040,
    0x0804020100804020,
    0x0402010080402010,
    0x0201008040201008,
    0x0100804020100804
};

// Move generator shift for ranks:
const int RANKSHIFT[64] = {
        1,  1,  1,  1,  1,  1,  1,  1,
        9,  9,  9,  9,  9,  9,  9,  9,
       17, 17, 17, 17, 17, 17, 17, 17,  
       25, 25, 25, 25, 25, 25, 25, 25,
       33, 33, 33, 33, 33, 33, 33, 33,
       41, 41, 41, 41, 41, 41, 41, 41,
       49, 49, 49, 49, 49, 49, 49, 49,
       57, 57, 57, 57, 57, 57, 57, 57
};

void init() {
    init_data();
    parse_fen(TEST_FEN);
}

int parse_fen(char *fen) {
  int file, rank, piece;
  reset_board();

  rank = 8;
  file = 1;
  while((rank >= 1) && *fen) {
    switch (*fen) {
      case 'p': piece = B_PAWN; 
                board.b_pawns |= (1ULL << (FRtoSQ(file, rank)));
                break;
      case 'r': piece = B_ROOK;
                board.b_rooks |= (1ULL << (FRtoSQ(file, rank))); 
                break;
      case 'n': piece = B_KNIGHT;
                board.b_knights |= (1ULL << (FRtoSQ(file, rank))); 
                break;
      case 'b': piece = B_BISHOP;
                board.b_bishops |= (1ULL << (FRtoSQ(file, rank))); 
                break;
      case 'k': piece = B_KING;
                board.b_king |= (1ULL << (FRtoSQ(file, rank))); 
                break;
      case 'q': piece = B_QUEEN;
                board.b_queens |= (1ULL << (FRtoSQ(file, rank))); 
                break;
      case 'P': piece = W_PAWN;
                board.w_pawns |= (1ULL << (FRtoSQ(file, rank))); 
                break;
      case 'R': piece = W_ROOK;
                board.w_rooks |= (1ULL << (FRtoSQ(file, rank))); 
                break;
      case 'N': piece = W_KNIGHT;
                board.w_knights |= (1ULL << (FRtoSQ(file, rank))); 
                break;
      case 'B': piece = W_BISHOP;
                board.w_bishops |= (1ULL << (FRtoSQ(file, rank))); 
                break;
      case 'K': piece = W_KING; 
                board.w_king |= (1ULL << (FRtoSQ(file, rank))); 
                break;
      case 'Q': piece = W_QUEEN;
                board.w_queens |= (1ULL << (FRtoSQ(file, rank))); 
                break;

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
                piece = EMPTY;
                file += *fen - '0';
                break;

      case '/':
      case ' ':
                rank--;
                file = 1;
                fen++;
                continue;              

      default:
                printf("FEN error \n");
                return -1;
    }   

    if (piece != EMPTY) {
      board.sq[FRtoSQ(file, rank)] = piece;
      file += 1;
    }

    fen++;
  }

  board.w_pieces = board.w_king | board.w_queens | board.w_rooks 
                   | board.w_bishops | board.w_knights | board.w_pawns; 

  board.b_pieces = board.b_king | board.b_queens | board.b_rooks 
                   | board.b_bishops | board.b_knights | board.b_pawns; 

  board.all_pieces = board.b_pieces | board.w_pieces;

  //debugging added to be certain the FEN board is correctly traversed
  assert(*fen == 'w' || *fen == 'b');

  board.side = (*fen == 'w')? WHITE : BLACK;
  fen += 2;

  while (*fen != ' ') {
    switch (*fen) {
      case 'K': board.castle_perm |= WKCA; break;
      case 'Q': board.castle_perm |= WQCA; break;
      case 'k': board.castle_perm |= BKCA; break;
      case 'q': board.castle_perm |= BQCA; break;
      default: break;
    }
    fen++;
  }
  fen++;

  //check that we didn't somehow screw up castle permissions
  assert((board.castle_perm >= 0b0000) && (board.castle_perm <= 0b1111));

  if (*fen != '-') {
    file = fen[0] - 'a';
    file = fen[1] - '1';

    assert(board.side == WHITE || (rank == 6 && (file >= 1 && file <= 8)));
    assert(board.side == BLACK || (rank == 3 && (file >= 1 && file <= 8)));

    board.ep_sq = FRtoSQ(file, rank);
  }

  return 0;
}

void reset_board() {
  int i;

  board.w_king = 0ULL;
  board.w_queens = 0ULL;
  board.w_rooks = 0ULL;
  board.w_bishops = 0ULL;
  board.w_knights = 0ULL;
  board.w_pawns = 0ULL;

  board.b_king = 0ULL;
  board.b_queens = 0ULL;
  board.b_rooks = 0ULL;
  board.b_bishops = 0ULL;
  board.b_knights = 0ULL;
  board.b_pawns = 0ULL;

  board.side = 0;
  board.castle_perm = 0;

  board.ep_sq = 0;
  board.fifty_move_count = 0;

  for (i = 0; i < 64; i++) {
    board.sq[i] = 0;
  }
}

void print_board() {
  int file, rank;

  for (rank = 8; rank >= 1; rank--) {
    printf("  +---+---+---+---+---+---+---+---+\n%d |", rank);
    for (file = 1; file <= 8; file++) {
      printf("%2c |", PIECE_NAME[board.sq[FRtoSQ(file, rank)]]);
    }
    printf("\n");
  }

  printf("  +---+---+---+---+---+---+---+---+\n");
  printf("    a   b   c   d   e   f   g   h  \n");

  printf("side:%c, castle_perm:%d, ep:%i, fifty_move:%i\n",
      (board.side)?'B':'W', board.castle_perm, board.ep_sq,
      board.fifty_move_count);
}

/* This is a help mehtod only used in debugging */
int debug_board() {
    int i;

    for (i = A1; i <= H8; i++) {
        if (board.sq[i] == B_PAWN) {
            assert((1ULL << i) & board.b_pawns);
            assert((1ULL << i) & board.b_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.b_pawns));
        }

        if (board.sq[i] == B_ROOK) {
            assert((1ULL << i) & board.b_rooks);
            assert((1ULL << i) & board.b_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.b_rooks));
        }

        if (board.sq[i] == B_KNIGHT) {
            assert((1ULL << i) & board.b_knights);
            assert((1ULL << i) & board.b_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.b_knights));
        }

        if (board.sq[i] == B_BISHOP) {
            assert((1ULL << i) & board.b_bishops);
            assert((1ULL << i) & board.b_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.b_bishops));
        }

        if (board.sq[i] == B_QUEEN) {
            assert((1ULL << i) & board.b_queens);
            assert((1ULL << i) & board.b_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.b_queens));
        }

        if (board.sq[i] == B_KING) {
            assert((1ULL << i) & board.b_king);
            assert((1ULL << i) & board.b_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.b_king));
        }

        if (board.sq[i] == W_PAWN) {
            assert((1ULL << i) & board.w_pawns);
            assert((1ULL << i) & board.w_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.w_pawns));
        }

        if (board.sq[i] == W_ROOK) {
            assert((1ULL << i) & board.w_rooks);
            assert((1ULL << i) & board.w_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.w_rooks));
        }

        if (board.sq[i] == W_KNIGHT) {
            assert((1ULL << i) & board.w_knights);
            assert((1ULL << i) & board.w_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.w_knights));
        }

        if (board.sq[i] == W_BISHOP) {
            assert((1ULL << i) & board.w_bishops);
            assert((1ULL << i) & board.w_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.w_bishops));
        }

        if (board.sq[i] == W_QUEEN) {
            assert((1ULL << i) & board.w_queens);
            assert((1ULL << i) & board.w_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.w_queens));
        }

        if (board.sq[i] == W_KING) {
            assert((1ULL << i) & board.w_king);
            assert((1ULL << i) & board.w_pieces);
            assert((1ULL << i) & board.all_pieces);
        } else {
            assert(!((1ULL << i) & board.w_king));
        }
    }

    return 1;
}

void print_bitboard(BIT_BOARD *bboard) {
    int i;

    for (i = 7; i >= 0; i--) {
        print_bitboard_rank(bboard->rank[i]);
    }
}

void print_bitboard_rank(uint8_t rank) {
    int i;
    for (i = 0; i < 8; i++) {
        printf("%d ", ((1 << i) & rank)? 1 : 0);
    }

    printf("\n");
}

