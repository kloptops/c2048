
#ifndef _C2048_H__
#define _C2048_H__

#pragma once

#include "stdafx.h"

#include "tinyrand.h"

/* our 2048 structures */
#define BOARD_SIZE 4
#define BOARD_MAX (BOARD_SIZE * BOARD_SIZE)

#define MOVE_UP    0
#define MOVE_DOWN  1
#define MOVE_LEFT  2
#define MOVE_RIGHT 3
#define MOVE_FIRST 0
#define MOVE_MAX   4

typedef struct _c2048_ctx
{
	struct _c2048_ctx *_next;
	uint32_t score;
	uint32_t board[BOARD_MAX];
	rand_ctx b_rand;
} c2048_ctx;

/* Initialize our 2048 game board using seed. */
void c2048_init(c2048_ctx *ctx, uint32_t seed);

/* Copy the board from 'ctx_from' to 'ctx_to' */
void c2048_copy(c2048_ctx *ctx_to, c2048_ctx *ctx_from);

/* Check to see if the move in 'direction' is possible. */
int c2048_can_move(c2048_ctx *ctx, int direction);

/* Do move in 'direction'. */
uint32_t c2048_do_move(c2048_ctx *ctx, int direction);

/* Add a tile to the board in a random place.
 * 90% chance of 2, 10% of being a 4.
 */
void c2048_add_tile(c2048_ctx *ctx);

/* Returns 1 if there are no free tiles, otherwise returns 0. */
int c2048_is_full(c2048_ctx *ctx);

/* Returns the number of empty tiles on the board. */
uint32_t c2048_empty_tiles(c2048_ctx *ctx);

/* Checks to see if no moves are possible.
*
* This function is designed to just figure out if ANY moves are possible.
* Use c2048_can_move(ctx, direction) to tell if a particular direction is possible.
*/
int c2048_no_moves(c2048_ctx *ctx);

/* Dumps the board to STDOUT. */
void c2048_print(c2048_ctx *ctx);

/* Find the largest value on the board currently. */
uint32_t c2048_max_value(c2048_ctx *ctx);

uint32_t c2048_find_furthest(c2048_ctx *ctx, uint32_t start, uint32_t end, uint32_t step);

#define c2048_pos(x, y) ((x) + ((y) * BOARD_SIZE))
#define c2048_addcol(pos) ((pos) + 1)
#define c2048_subcol(pos) ((pos) - 1)
#define c2048_addrow(pos) ((pos) + BOARD_SIZE)
#define c2048_subrow(pos) ((pos) - BOARD_SIZE)

#endif // _C2048_H__
