
#ifndef _C2048_H__
#define _C2048_H__

#pragma once

#include "stdafx.h"

#include "tinyrand.h"

/* our 2048 structures */
#define BOARD_SIZE 4
#define MAX_BOARD (BOARD_SIZE * BOARD_SIZE)
#define MOVE_UP    0
#define MOVE_DOWN  1
#define MOVE_LEFT  2
#define MOVE_RIGHT 3
#define MOVE_FIRST 0
#define MOVE_MAX 4

typedef struct _c2048_ctx
{
	struct _c2048_ctx *_next;
	uint32_t score;
	uint16_t board[MAX_BOARD];
	rand_ctx b_rand;
} c2048_ctx;

void c2048_init(c2048_ctx *ctx, uint32_t seed);
void c2048_copy(c2048_ctx *ctx_to, c2048_ctx *ctx_from);
int c2048_can_move(c2048_ctx *ctx, int direction);
uint32_t c2048_do_move(c2048_ctx *ctx, int direction);
void c2048_add_tile(c2048_ctx *ctx);
int c2048_is_full(c2048_ctx *ctx);
uint32_t c2048_empty_cells(c2048_ctx *ctx);
int c2048_no_moves(c2048_ctx *ctx);
void c2048_print(c2048_ctx *ctx);
uint32_t c2048_find_furthest(c2048_ctx *ctx, uint32_t start, uint32_t end, uint32_t step);
uint16_t c2048_max_value(c2048_ctx *ctx);

#define c2048_pos(x, y) ((x) + ((y) * BOARD_SIZE))
#define c2048_addcol(pos) ((pos) + 1)
#define c2048_subcol(pos) ((pos) - 1)
#define c2048_addrow(pos) ((pos) + BOARD_SIZE)
#define c2048_subrow(pos) ((pos) - BOARD_SIZE)

#endif // _C2048_H__
