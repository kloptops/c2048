
#include "stdafx.h"

#include "tinyrand.h"


/* Random number generator */
#define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
uint32_t rand_val(rand_ctx *x)
{
	uint32_t e = x->a - rot(x->b, 27);
	x->a = x->b ^ rot(x->c, 17);
	x->b = x->c + x->d;
	x->c = x->d + e;
	x->d = e + x->a;
	return x->d;
}

void rand_init(rand_ctx *x, uint32_t seed)
{
	uint32_t i;
	x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
	for (i = 0; i < 20; ++i) {
		(void)rand_val(x);
	}
}
