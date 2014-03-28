
#pragma once

#include "stdafx.h"


/* A small noncryptographic PRNG
*
* http://burtleburtle.net/bob/rand/smallprng.html
*/

typedef struct _rand_ctx
{
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
} rand_ctx;

uint32_t rand_val(rand_ctx *x);
void rand_init(rand_ctx *x, uint32_t seed);
