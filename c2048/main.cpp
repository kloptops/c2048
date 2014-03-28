#include "stdafx.h"

#include "tinyrand.h"
#include "c2048.h"

const char *move_to_str[] = { "UP", "DOWN", "LEFT", "RIGHT" };

int main(int argc, char **argv)
{
	c2048_ctx board;
	int moves = 0, last_moves;

	c2048_init(&board, time(NULL) + clock());

	board.board[0] = 2;

	c2048_print(&board);

	while (!c2048_no_moves(&board) && moves < 10000)
	{
		last_moves = moves;
		if (c2048_can_move(&board, MOVE_LEFT))
		{
			moves += 1;
			c2048_do_move(&board, MOVE_LEFT);
		}
		if (c2048_can_move(&board, MOVE_UP))
		{
			moves += 1;
			c2048_do_move(&board, MOVE_UP);
		}
		if (c2048_can_move(&board, MOVE_RIGHT))
		{
			moves += 1;
			c2048_do_move(&board, MOVE_RIGHT);
		}
		if (c2048_can_move(&board, MOVE_UP))
		{
			moves += 1;
			c2048_do_move(&board, MOVE_UP);
		}
		if (last_moves == moves)
			c2048_do_move(&board, MOVE_DOWN);
	}

	printf("Score: %d\n", board.score);
	c2048_print(&board);

	return 0;
}
