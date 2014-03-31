
#include "stdafx.h"
#include "c2048.h"
#include "c2048_ai.h"

c2048_ai_ctx *c2048_ai_create(uint32_t seed)
{
	c2048_ai_ctx *ai_ctx;

	ai_ctx = (c2048_ai_ctx*)calloc(1, sizeof(c2048_ai_ctx));
	if (ai_ctx == NULL)
		return NULL;

	ai_ctx->current_board = (c2048_ctx*)calloc(1, sizeof(c2048_ctx));
	if (ai_ctx->current_board == NULL)
	{
		free(ai_ctx);
		return NULL;
	}

	ai_ctx->total_free_boards = 0;

	ai_ctx->score_weight = 1.0;
	ai_ctx->mono2_weight = 2.f;
	ai_ctx->smooth_weight = -0.1f;
	ai_ctx->free_weight = 2.7;
	ai_ctx->max_weight = 0.5;
	ai_ctx->no_moves_weight = -3000.f;
	ai_ctx->rowscore_weight = 0.5;

	c2048_init(ai_ctx->current_board, seed);

	return ai_ctx;
}

c2048_ctx *c2048_ai_board_push(c2048_ai_ctx *ai_ctx)
{
	c2048_ctx *new_ctx, *old_ctx;

	if (ai_ctx->free_boards != NULL)
	{
		new_ctx = ai_ctx->free_boards;
		ai_ctx->free_boards = new_ctx->_next;
		ai_ctx->total_free_boards -= 1;
	}
	else
	{
		new_ctx = (c2048_ctx *)calloc(1, sizeof(c2048_ctx));
	}

	old_ctx = ai_ctx->current_board;
	ai_ctx->current_board = new_ctx;

	c2048_copy(new_ctx, old_ctx);

	new_ctx->_next = old_ctx;

	return new_ctx;
}

c2048_ctx *c2048_ai_board_pop(c2048_ai_ctx *ai_ctx, int keep)
{
	c2048_ctx *new_ctx, *old_ctx;

	old_ctx = ai_ctx->current_board;
	if (old_ctx->_next == NULL)
	{
		printf("AI board popped too far!");
		abort();
	}

	new_ctx = old_ctx->_next;
	if (keep != 0)
	{
		old_ctx->_next = new_ctx->_next;
		new_ctx->_next = ai_ctx->free_boards;
		ai_ctx->free_boards = new_ctx;
		ai_ctx->total_free_boards += 1;
	}
	else
	{
		old_ctx->_next = ai_ctx->free_boards;
		ai_ctx->free_boards = old_ctx;
		ai_ctx->current_board = new_ctx;
		ai_ctx->total_free_boards += 1;
	}

	return ai_ctx->current_board;
}

void c2048_ai_destroy(c2048_ai_ctx *ai_ctx)
{
	c2048_ctx *ctx, *next_ctx;

	ctx = ai_ctx->free_boards;
	while (ctx != NULL)
	{
		next_ctx = ctx->_next;
		free(ctx);
		ctx = next_ctx;
	}

	ctx = ai_ctx->current_board;
	while (ctx != NULL)
	{
		next_ctx = ctx->_next;
		free(ctx);
		ctx = next_ctx;
	}

	free(ai_ctx);
}


double c2048_ai_find_moves(c2048_ai_ctx *ai_ctx, c2048_move_chain *move_chain, int depth, int max_depth)
{
	int direction;
	double score, best_score = 0.f;
	int first_move = 1;
	c2048_move_chain *best_moves = NULL, *temp_chain;

	for (direction = MOVE_FIRST; direction < MOVE_MAX; direction++)
	{
		if (!c2048_can_move(ai_ctx->current_board, direction))
			continue;

		score = 0.0f;
		temp_chain = c2048_move_chain_create(direction, score);
		if (temp_chain == NULL)
			continue;

		score = c2048_ai_rate_move(ai_ctx, direction);
		if (depth < max_depth)
		{
			(void)c2048_ai_board_push(ai_ctx);
			c2048_do_move(ai_ctx->current_board, direction);

			if (!c2048_no_moves(ai_ctx->current_board))
				score += c2048_ai_find_moves(ai_ctx, temp_chain, depth + 1, max_depth);
			(void)c2048_ai_board_pop(ai_ctx, 0);
		}

		temp_chain->score = score;

		if (first_move == 1 || score > best_score)
		{
			first_move = 0;
			best_score = score;
			if (best_moves != NULL)
				c2048_move_chain_destroy(best_moves);

			best_moves = temp_chain;
		}
		else
			c2048_move_chain_destroy(temp_chain);
	}

	if (first_move == 1 || best_moves == NULL)
		return 0.0f;

	c2048_move_chain_append_chain(move_chain, best_moves);
	return best_score;
}

double c2048_ai_rate_move(c2048_ai_ctx *ai_ctx, int direction)
{
	c2048_ctx *ctx;
	double score = 0.0f;

	ctx = c2048_ai_board_push(ai_ctx);

	score += (double)c2048_do_move(ctx, direction) * ai_ctx->score_weight;

	if (ai_ctx->smooth_weight != 0.0)
		score += c2048_ai_calc_smoothness(ai_ctx) * ai_ctx->smooth_weight;

	if (!c2048_is_full(ctx) && ai_ctx->free_weight != 0.0)
		score += log((double)c2048_empty_cells(ctx)) * ai_ctx->free_weight;

	if (ai_ctx->mono2_weight != 0.0)
		score += c2048_ai_calc_monotonicity2(ai_ctx) * ai_ctx->mono2_weight;

	if (ai_ctx->max_weight != 0.0)
		score += (double)c2048_max_value(ctx) * ai_ctx->max_weight;

	if (ai_ctx->no_moves_weight != 0.0)
		score += (double)c2048_no_moves(ai_ctx->current_board) * ai_ctx->no_moves_weight;

	if (ai_ctx->rowscore_weight != 0.0)
		score += (double)c2048_ai_calc_rowscore(ai_ctx) * ai_ctx->rowscore_weight;

	(void)c2048_ai_board_pop(ai_ctx, 0);

	return score;
}

// measures how smooth the grid is (as if the values of the pieces
// were interpreted as elevations). Sums of the pairwise difference
// between neighboring tiles (in log space, so it represents the
// number of merges that need to happen before they can merge). 
// Note that the pieces can be distant
double c2048_ai_calc_smoothness(c2048_ai_ctx *ai_ctx)
{
	uint32_t x, y;
	uint32_t pos, pos2;

	c2048_ctx *ctx = ai_ctx->current_board;

	double value, _log2, smoothness = 0.0f;

	_log2 = log(2.f);

	for (x = 0; x < BOARD_SIZE; x++)
	{
		for (y = 0; y < BOARD_SIZE; y++)
		{
			pos = c2048_pos(x, y);
			if (ctx->board[pos] != 0)
			{
				value = log((double)ctx->board[pos]) / _log2;
				pos2 = c2048_find_furthest(ctx, pos, c2048_pos(BOARD_SIZE, y), 1);
				if (pos2 != pos && ctx->board[pos2] != 0)
				{
					smoothness += fabs(value - (log((double)ctx->board[pos2]) / _log2));
				}

				pos2 = c2048_find_furthest(ctx, pos, c2048_pos(x, BOARD_SIZE), BOARD_SIZE);
				if (pos2 != pos && ctx->board[pos2] != 0)
				{
					smoothness += fabs(value - (log((double)ctx->board[pos2]) / _log2));
				}
			}
		}
	}

	return smoothness;
}

#if BOARD_SIZE == 4
static const uint32_t _rowscore_weight[16] = {
	16, 15, 14, 13,
	 9, 10, 11, 12,
	 8,  7,  6,  5,
	 4,  3,  2,  1,
	};
#elif BOARD_SIZE == 5
static const uint32_t _rowscore_weight[25] = {
	25, 24, 23, 22, 21,
	16, 17, 18, 19, 20,
	15, 14, 13, 12, 11,
	 6,  7,  8,  9, 10,
	 5,  4,  3,  2,  1,
	};
#else
#error "Run make_rowscore_weights.py"
#endif

uint32_t c2048_ai_calc_rowscore(c2048_ai_ctx *ai_ctx)
{
	uint32_t score = 0, i, *board;

	board = ai_ctx->current_board->board;

	for (i = 0; i < MAX_BOARD; i++)
		score += board[i] * _rowscore_weight[i];

	return score;
}


#define _MAX(a, b) (((a) > (b)) ? (a) : (b))

// measures how monotonic the grid is. This means the values of the tiles are strictly increasing
// or decreasing in both the left/right and up/down directions
double c2048_ai_calc_monotonicity2(c2048_ai_ctx *ai_ctx)
{
	// scores for all four directions
	double totals[4] = { 0, 0, 0, 0 };
	uint32_t current, next, x, y;
	c2048_ctx *ctx;
	double current_value, next_value, _log2;

	_log2 = log(2.0f);

	ctx = ai_ctx->current_board;

	// up/down direction
	for (x = 0; x < MAX_BOARD; x++)
	{
		current = 0;
		next = current + 1;
		while (next < 4)
		{
			while (next < 4 && ctx->board[c2048_pos(x, next)] == 0)
				next++;

			if (next >= 4)
				next--;

			current_value = ((ctx->board[c2048_pos(x, current)] != 0) ? (log((double)ctx->board[c2048_pos(x, current)]) / _log2) : (0.f));
			next_value = ((ctx->board[c2048_pos(x, next)] != 0) ? (log((double)ctx->board[c2048_pos(x, next)]) / _log2) : (0.f));
			if (current_value > next_value)
			{
				totals[MOVE_UP] += next_value - current_value;
			}
			else if (next_value > current_value) {
				totals[MOVE_DOWN] += current_value - next_value;
			}
			current = next;
			next++;
		}
	}

	// left/right direction
	for (y = 0; y < MAX_BOARD; y++)
	{
		current = 0;
		next = current + 1;
		while (next < 4)
		{
			while (next < 4 && ctx->board[c2048_pos(next, y)] == 0)
				next++;

			if (next >= 4)
				next--;

			current_value = ((ctx->board[c2048_pos(current, y)] != 0) ? (log((double)ctx->board[c2048_pos(current, y)]) / _log2) : (0.f));
			next_value = ((ctx->board[c2048_pos(next, y)] != 0) ? (log((double)ctx->board[c2048_pos(next, y)]) / _log2) : (0.f));

			if (current_value > next_value)
			{
				totals[MOVE_LEFT] += next_value - current_value;
			}
			else if (next_value > current_value) {
				totals[MOVE_RIGHT] += current_value - next_value;
			}

			current = next;
			next++;
		}
	}

	return _MAX(totals[MOVE_UP], totals[MOVE_RIGHT]) + _MAX(totals[MOVE_DOWN], totals[MOVE_LEFT]);
}

// TODO: Make a parent allocator that can juggle allocation better :)
static c2048_move_chain *_global_chain = NULL;
static uint32_t _global_chain_length = 0;
static uint32_t _global_chain_allocs = 0;
static uint32_t _global_chain_frees = 0;
static uint32_t _global_chain_recycled = 0;

#define MAX_GLOBAL_CHAIN 1024

void c2048_move_chain_statistics()
{
	printf("----- MOVE CHAIN STATS -----\n");
	printf("Global Chain Length: %d\n", _global_chain_length);
	printf("Chain Allocs:\n");
	printf("  Allocs:   %7d\n", _global_chain_allocs);
	printf("  Frees:    %7d\n", _global_chain_frees);
	printf("  Recycled: %7d\n", _global_chain_recycled);
	printf("\n");
}


c2048_move_chain *c2048_move_chain_create(int direction, double score)
{
	c2048_move_chain *chain;

	if (_global_chain != NULL)
	{
		chain = _global_chain;
		_global_chain = chain->next;
		_global_chain_length -= 1;

		memset(chain, 0, sizeof(c2048_move_chain));
		_global_chain_recycled += 1;
	}
	else
	{
		chain = (c2048_move_chain*)calloc(1, sizeof(c2048_move_chain));
		if (chain == NULL)
			return NULL;
		_global_chain_allocs += 1;
	}

	chain->direction = direction;
	chain->score = 0.0f;

	return chain;
}

void c2048_move_chain_append(c2048_move_chain *chain, int direction, double score)
{
	c2048_move_chain *new_chain;

	new_chain = c2048_move_chain_create(direction, score);
	if (new_chain == NULL)
		return;

	c2048_move_chain_append_chain(chain, new_chain);
}

void c2048_move_chain_append_chain(c2048_move_chain *chain, c2048_move_chain *new_chain)
{
	c2048_move_chain *temp_chain;

	temp_chain = chain;
	while (temp_chain->next != NULL)
		temp_chain = temp_chain->next;

	temp_chain->next = new_chain;
}

void c2048_move_chain_destroy(c2048_move_chain *chain)
{
	c2048_move_chain *temp_chain;

	while (chain != NULL)
	{
		temp_chain = chain->next;

		if (_global_chain_length < MAX_GLOBAL_CHAIN)
		{
			chain->next = _global_chain;
			_global_chain = chain;
			_global_chain_length += 1;
		}
		else
		{
			free(chain);
			_global_chain_frees += 1;
		}

		chain = temp_chain;
	}
}

c2048_move_chain *c2048_move_chain_next(c2048_move_chain *chain)
{
	c2048_move_chain *next;

	if (chain == NULL)
		return NULL;

	next = chain->next;

	if (_global_chain_length < MAX_GLOBAL_CHAIN)
	{
		chain->next = _global_chain;
		_global_chain = chain;
		_global_chain_length += 1;
	}
	else
	{
		free(chain);
		_global_chain_frees += 1;
	}

	return next;
}

uint32_t c2048_move_chain_length(c2048_move_chain *chain)
{
	uint32_t length = 0;

	while (chain != NULL)
	{
		length += 1;
		chain = chain->next;
	}

	return length;
}
