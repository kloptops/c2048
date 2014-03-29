
#pragma once

#include "stdafx.h"
#include "c2048.h"

typedef struct _c2048_ai_ctx
{
	c2048_ctx *free_boards;
	c2048_ctx *current_board;
	uint32_t total_free_boards;
} c2048_ai_ctx;

typedef struct _c2048_move_chain
{
	struct _c2048_move_chain *next;
	int direction;
	float score;
} c2048_move_chain;


c2048_ai_ctx *c2048_ai_create(uint32_t seed);
c2048_ctx *c2048_ai_board_push(c2048_ai_ctx *ai_ctx);
c2048_ctx *c2048_ai_board_pop(c2048_ai_ctx *ai_ctx, int keep);
void c2048_ai_destroy(c2048_ai_ctx *ai_ctx);

float c2048_ai_rate_move(c2048_ai_ctx *ai_ctx, int direction);

float c2048_ai_calc_smoothness(c2048_ai_ctx *ai_ctx);
float c2048_ai_calc_monotonicity2(c2048_ai_ctx *ai_ctx);

float c2048_ai_find_moves(c2048_ai_ctx *ai_ctx, c2048_move_chain *move_chain, int depth, int max_depth);

// Not thread safe :(
c2048_move_chain *c2048_move_chain_create(int direction, float score);
void c2048_move_chain_append(c2048_move_chain *chain, int direction, float score);
void c2048_move_chain_append_chain(c2048_move_chain *chain, c2048_move_chain *new_chain);
void c2048_move_chain_destroy(c2048_move_chain *chain);

c2048_move_chain *c2048_move_chain_next(c2048_move_chain *chain);
uint32_t c2048_move_chain_length(c2048_move_chain *chain);

void c2048_move_chain_statistics();