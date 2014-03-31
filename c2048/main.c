#include "stdafx.h"

#include "tinyrand.h"
#include "c2048.h"
#include "c2048_ai.h"

const char *move_to_str[] = { "UP", "DOWN", "LEFT", "RIGHT" };
const int adjacent_direction[] = {MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN};

#define AI_LOOK_MOVES 8
#define AI_DO_MOVES 3
#define DO_PRINT 1

// #define PLAY_SEED 1396182386

int main(int argc, char **argv)
{
	c2048_ai_ctx *ai;
	c2048_move_chain *move_chain;
	int moves = 0, done_moves, moves_threshold;
	double best_score;
	uint32_t moves_left;

#ifdef PLAY_SEED
	uint32_t seed = PLAY_SEED;
#else
	uint32_t seed = time(NULL) + clock();
#endif

    ai = c2048_ai_create(seed);
	// ai = c2048_ai_create(10);

	printf("Seed: %d\n", seed);

	c2048_print(ai->current_board);

	ai->smooth_weight = 0.0f;
	ai->mono2_weight = 0.0f;

	while (!c2048_no_moves(ai->current_board))
	{
		// Add a bogus move
		move_chain = c2048_move_chain_create(100, 0.0f);

		best_score = c2048_ai_find_moves(ai, move_chain, 0, AI_LOOK_MOVES);

		// Skip our bogus move
		move_chain = c2048_move_chain_next(move_chain);

		if (move_chain == NULL)
			continue;

		moves_threshold = c2048_move_chain_length(move_chain) >= (AI_LOOK_MOVES - AI_DO_MOVES);
		done_moves = 0;

		if (DO_PRINT)
			printf("\nAI Calculated Moves:\n");

		while (move_chain != NULL)
		{
			if (moves_threshold != 0 && done_moves >= AI_DO_MOVES)
				break;

			done_moves += 1;

			if (DO_PRINT)
				printf("\n%-10s (%0.2f / %0.2f)\n", move_to_str[move_chain->direction], move_chain->score, best_score);

			c2048_do_move(ai->current_board, move_chain->direction);

			moves += 1;

			move_chain = c2048_move_chain_next(move_chain);
		}

		c2048_move_chain_destroy(move_chain);

		if (DO_PRINT)
			c2048_print(ai->current_board);
	}

	if (DO_PRINT)
		printf("\n----------------------------\n");

	if (DO_PRINT)
		printf("Seed:  %d\n", seed);

	printf("Score: %d\n", ai->current_board->score);
	c2048_print(ai->current_board);

	printf("\n");

	// c2048_move_chain_statistics();

	return 0;
}
