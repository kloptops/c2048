#include "stdafx.h"

#include "tinyrand.h"
#include "c2048.h"

#define _c2048_rand_ctx(ctx) (&(ctx->b_rand))

void c2048_init(c2048_ctx *ctx, uint32_t seed)
{
	if (ctx == NULL)
		return;

	memset(ctx, 0, sizeof(c2048_ctx));

	rand_init(_c2048_rand_ctx(ctx), seed);

	c2048_add_tile(ctx);
	c2048_add_tile(ctx);
}

void c2048_copy(c2048_ctx *ctx_to, c2048_ctx *ctx_from)
{
	if (ctx_to == NULL || ctx_from == NULL)
		return;

	memcpy(ctx_to, ctx_from, sizeof(c2048_ctx));
}

void c2048_print(c2048_ctx *ctx)
{
	uint32_t i;

	if (ctx == NULL)
		return;

	for (i = 0; i < BOARD_MAX; i++)
	{
		if ((i % BOARD_SIZE) == 0)
			printf("  ");
		printf(" %5d", ctx->board[i]);
		if ((i % BOARD_SIZE) == (BOARD_SIZE - 1))
			printf("\n");
	}
}

uint32_t c2048_empty_tiles(c2048_ctx *ctx)
{
	uint32_t i, counter=0;

	if (ctx == NULL)
		return 0;

	for (i = 0; i < BOARD_MAX; i++)
		if (ctx->board[i] == 0)
			counter += 1;

	return counter;
}

int c2048_is_full(c2048_ctx *ctx)
{
	uint32_t i;

	if (ctx == NULL)
		return 1;

	for (i = 0; i < BOARD_MAX; i++)
	if (ctx->board[i] == 0)
		return 0;

	return 1;
}


int c2048_no_moves(c2048_ctx *ctx)
{
	uint32_t x, y, pos;
	uint32_t tile;

	for (y = 0; y < BOARD_SIZE; y++)
	{
		for (x = 0; x < BOARD_SIZE; x++)
		{
			pos = c2048_pos(x, y);
			tile = ctx->board[pos];
			if (tile == 0)
				return 0;

			if (x < BOARD_SIZE - 1 && tile == ctx->board[c2048_addcol(pos)])
				return 0;
			if (y < BOARD_SIZE - 1 && tile == ctx->board[c2048_addrow(pos)])
				return 0;
		}
	}

	return 1;
}

int c2048_can_move(c2048_ctx *ctx, int direction)
{
	uint32_t x, y, pos;
	uint32_t tile, *board;

	board = (uint32_t*)(ctx->board);

	if (direction == MOVE_LEFT || direction == MOVE_RIGHT)
	{
		for (y = 0; y < BOARD_SIZE; y++)
		{
			for (x = 0; x < BOARD_SIZE; x++)
			{
				pos = c2048_pos(x, y);
				tile = board[pos];
				if (x < BOARD_SIZE - 1 &&
					tile != 0 &&
					tile == board[c2048_addcol(pos)])
					return 1;
				if (direction == MOVE_RIGHT &&
					x > 0 &&
					tile == 0 &&
					board[c2048_subcol(pos)] != 0)
					return 1;
				if (direction == MOVE_LEFT &&
					x < BOARD_SIZE - 1 &&
					tile == 0 &&
					board[c2048_addcol(pos)] != 0)
					return 1;
			}
		}
	}
	else if (direction == MOVE_UP || direction == MOVE_DOWN)
	{
		for (x = 0; x < BOARD_SIZE; x++)
		{
			for (y = 0; y < BOARD_SIZE; y++)
			{
				pos = c2048_pos(x, y);
				tile = board[pos];
				if (y < BOARD_SIZE - 1 &&
					tile != 0 &&
					tile == board[c2048_addrow(pos)])
					return 1;
				if (direction == MOVE_DOWN &&
					y > 0 &&
					tile == 0 &&
					board[c2048_subrow(pos)] != 0)
					return 1;
				if (direction == MOVE_UP &&
					y < BOARD_SIZE - 1 &&
					tile == 0 &&
					board[c2048_addrow(pos)] != 0)
					return 1;
			}
		}
	}
	return 0;
}

/* This function handles collapsing adjacent matching tiles.
 *
 * It iterates over the board using start, end & step. This controls whether we are working
 * on a left/right or up/down direction. start should always be less than end, and step should
 * always be positive.
 *
 * One side effect of this tile is that after two tiles are collapsed, we insert a 0 tile.
 * Thus a second call to _c2048_do_move is required to fill the gap.
 */
uint32_t _c2048_do_collapse(c2048_ctx *ctx, int start, int end, int step)
{
	int i, score = 0;
	uint32_t *board, *tile;

	board = (uint32_t*)(ctx->board);

	for (i = start; i < end; i += step)
	{
		tile = &(board[i]);
		if (*tile == 0)
			continue;

		if (*tile == board[i + step])
		{
			*tile *= 2;
			board[i + step] = 0;
			score += *tile;
		}
	}

	return score;
}

/* This function handles moving the tiles.
 *
 * It iterates over the board using start, end & step. This controls whether we are working
 * on a left/right or up/down direction. start should always be less than end, and step should
 * always be positive.
 *
 * We handle down and right condensing by using a buffer that is twice the size of the board.
 * We pack all the tiles into the end of the buffer then use a simple hack to make down and
 * right.
 *
 * Start:
 *   input[4]  = {0, 2, 2, 0};
 *   buffer[8] = {0, 0, 0, 0, 0, 0, 0};
 *
 * Condensed:
 *   buffer[8] = {0, 0, 0, 0, 2, 2, 0, 0};
 *   tile_counter = 2; // 2 tiles found.
 *
 * Output for left:
 * buffer[8] = {0, 0, 0, 2, 2, 0, 0};
 *                       ^- start here
 * output[4] = {2, 2, 0, 0};
 *
 * Output for right:
 * buffer[8] = {0, 0, 0, 0, 2, 2, 0, 0};
 *                    ^- start here.
 * output[4] = {0, 0, 2, 2};
 *
 * We use tile_counter to get the right direction offset.
 */
void _c2048_do_move(c2048_ctx *ctx, int start, int end, int step, int direction)
{
	int i, pos;
	uint32_t *board;
	uint32_t _tiles[BOARD_SIZE * 2], *tiles, tile_counter = 0;

	for (i = 0; i < BOARD_SIZE * 2; i++)
		_tiles[i] = 0;

	board = (uint32_t*)(ctx->board);
	tiles = (uint32_t*)&(_tiles[BOARD_SIZE]);

	for (pos = start; pos <= end; pos += step)
	{
		if (board[pos] == 0)
			continue;
		tiles[tile_counter++] = board[pos];
	}

	if (tile_counter == BOARD_SIZE || tile_counter == 0)
		return;

	/* HAX */
	if (direction == MOVE_DOWN || direction == MOVE_RIGHT)
		tiles = (uint32_t*)&(_tiles[tile_counter]);

	tile_counter = 0;
	for (pos = start; pos <= end; pos += step)
	{
		board[pos] = tiles[tile_counter++];
	}
}

uint32_t c2048_do_move(c2048_ctx *ctx, int direction)
{
	uint32_t score = 0, i;
	uint32_t start, end, step;
	uint32_t tiles[BOARD_MAX];

	switch (direction)
	{
	case MOVE_RIGHT:
	case MOVE_LEFT:
		memcpy(tiles, ctx->board, sizeof(tiles));

		step = 1;
		for (i = 0; i < BOARD_SIZE; i++)
		{
			start = c2048_pos(0, i);
			end = c2048_pos(BOARD_SIZE - 1, i);

			_c2048_do_move(ctx, start, end, step, direction);
			score += _c2048_do_collapse(ctx, start, end, step);
			_c2048_do_move(ctx, start, end, step, direction);
		}
		break;

	case MOVE_DOWN:
	case MOVE_UP:
		memcpy(tiles, ctx->board, sizeof(tiles));

		step = BOARD_SIZE;
		for (i = 0; i < BOARD_SIZE; i++)
		{
			start = c2048_pos(i, 0);
			end = c2048_pos(i, BOARD_SIZE - 1);

			_c2048_do_move(ctx, start, end, step, direction);
			score += _c2048_do_collapse(ctx, start, end, step);
			_c2048_do_move(ctx, start, end, step, direction);
		}
		break;
	default:
		break;
	}

	if (memcmp(tiles, ctx->board, sizeof(tiles)) != 0)
		c2048_add_tile(ctx);

	ctx->score += score;

	return score;
}

uint32_t c2048_find_furthest(c2048_ctx *ctx, uint32_t start, uint32_t end, uint32_t step)
{
	uint32_t pos;

	for (pos = start; pos < end; pos += step)
	{
		if (ctx->board[pos] == 0)
			break;
	}

	return pos;
}

static const uint32_t _c2048_rand_values[10] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 4 };
#define _c2048_rand_value_total 10
void c2048_add_tile(c2048_ctx *ctx)
{
	uint32_t i;
	uint32_t rand_spots_total = 0;
	uint32_t rand_spots[BOARD_MAX];
	uint32_t rand_spot, rand_value;

	if (ctx == NULL)
		return;

	for (i = 0; i < BOARD_MAX; i++)
	if (ctx->board[i] == 0)
		rand_spots[rand_spots_total++] = i;

	if (rand_spots_total == 0)
		return;

	rand_spot = rand_spots[
		rand_val(_c2048_rand_ctx(ctx)) % rand_spots_total];
		rand_value = _c2048_rand_values[
			rand_val(_c2048_rand_ctx(ctx)) % _c2048_rand_value_total];

			ctx->board[rand_spot] = rand_value;
}

uint32_t c2048_max_value(c2048_ctx *ctx)
{
	uint32_t i;
	uint32_t max=0;
	for (i = 0; i < BOARD_SIZE; i++)
	{
		if (ctx->board[i] > max)
			max = ctx->board[i];
	}

	return max;
}
