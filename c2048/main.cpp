#include "stdafx.h"

#include "tinyrand.h"
#include "c2048.h"
#include "c2048_ai.h"

const char *move_to_str[] = { "UP", "DOWN", "LEFT", "RIGHT" };
const int adjacent_direction[] = {MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN};

int main(int argc, char **argv)
{
	c2048_ai_ctx *ai;
	c2048_move_chain *move_chain;
	int moves = 0, zeroes, z;
	float best_score, score;

    ai = c2048_ai_create(time(NULL) + clock());
	// ai = c2048_ai_create(10);

	c2048_print(ai->current_board);

	zeroes = 0;

	while (!c2048_no_moves(ai->current_board))
	{
		// Add a bogus move
		move_chain = c2048_move_chain_create(100, 0.0f);

		best_score = c2048_ai_find_moves(ai, move_chain, 0, 7);

		printf("AI Calculated Moves:");
		// Skip our bogus move
		move_chain = c2048_move_chain_next(move_chain);

		while (move_chain != NULL)
		{
			printf("\n%-10s (%0.2f)\n", move_to_str[move_chain->direction], move_chain->score);
			c2048_do_move(ai->current_board, move_chain->direction);

			moves += 1;

			move_chain = c2048_move_chain_next(move_chain);
		}

		c2048_print(ai->current_board);
	}

	printf("\n");

	printf("Score: %d\n", ai->current_board->score);
	c2048_print(ai->current_board);

	c2048_move_chain_statistics();

	return 0;
}
