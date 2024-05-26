#include <linux/slab.h>

#include "poly.h"

static uint8_t mymax(uint8_t a, uint8_t b)
{
	return a >= b ? a : b;
}

void poly_free(poly_t f)
{
	if (f != NULL)
		kfree(f->coef);
	kfree(f);
}

poly_t poly_init_from_array(uint8_t *arr, size_t n, uint8_t p)
{
	poly_t f;

	if (n == 0)
		return NULL;

	if ((f = kmalloc(sizeof(*f), GFP_KERNEL)) == NULL)
		return NULL;

	if ((f->coef = kmalloc(sizeof(*f->coef) * n, GFP_KERNEL)) == NULL) {
		poly_free(f);
		return NULL;
	}

	f->deg = n - 1;
	f->p = p;
	memcpy(f->coef, arr, sizeof(*f->coef) * n);
	poly_normalize(f);

	return f;
}

poly_t poly_copy(c_poly_t f)
{
	poly_t g;

	if (f == NULL)
		return NULL;

	if ((g = kmalloc(sizeof(*g), GFP_KERNEL)) == NULL)
		return NULL;

	g->deg = f->deg;
	g->p = f->p;
	g->coef = kmalloc(sizeof(*f->coef) * (f->deg + 1), GFP_KERNEL);
	if (g->coef == NULL)
		return g;

	memcpy(g->coef, f->coef, (f->deg + 1) * sizeof(*f->coef));

	return g;
}

void poly_normalize(poly_t f)
{
	uint8_t cnt;

	if (f == NULL)
		return;

	for (cnt = 0; cnt < f->deg; cnt++)
		if (f->coef[f->deg - cnt] != 0)
			break;

	f->deg -= cnt;
}

int poly_isequal(c_poly_t f, c_poly_t g)
{
	return f->deg == g->deg && f->p == g->p &&
	       !memcmp(f->coef, g->coef, f->deg + 1);
}

int poly_iszero(c_poly_t f)
{
	return f->deg == 0 && f->coef[0] == 0;
}

poly_t poly_neg(c_poly_t f)
{
	poly_t res;
	uint8_t i;

	if (f == NULL)
		return NULL;

	if ((res = kmalloc(sizeof(*res), GFP_KERNEL)) == NULL)
		return NULL;

	res->coef = kmalloc((f->deg + 1) * sizeof(*res->coef), GFP_KERNEL);
	if (res->coef == NULL)
		return NULL;

	res->deg = f->deg;
	res->p = f->p;
	for (i = 0; i <= f->deg; i++)
		res->coef[i] = (f->p - f->coef[i]) % f->p;

	poly_normalize(res);

	return res;
}

poly_t poly_sum(c_poly_t a, c_poly_t b)
{
	poly_t res;
	uint8_t maxdeg;
	uint8_t i;

	if (a->p != b->p)
		return NULL;

	if ((res = kmalloc(sizeof(*res), GFP_KERNEL)) == NULL)
		return NULL;

	maxdeg = mymax(a->deg, b->deg);
	res->coef = kcalloc(maxdeg + 1, sizeof(*res->coef), GFP_KERNEL);
	if (res->coef == NULL)
		return NULL;

	res->deg = maxdeg;
	res->p = a->p;

	for (i = 0; i <= maxdeg; i++) {
		if (i <= a->deg)
			res->coef[i] += a->coef[i];
		if (i <= b->deg)
			res->coef[i] += b->coef[i];
		res->coef[i] %= a->p;
	}

	poly_normalize(res);

	return res;
}

poly_t poly_subtract(c_poly_t a, c_poly_t b)
{
	poly_t b_neg, res;

	if (a->p != b->p)
		return NULL;

	b_neg = poly_neg(b);
	res = poly_sum(a, b_neg);
	poly_free(b_neg);

	return res;
}

poly_t poly_multiply(c_poly_t a, c_poly_t b)
{
	poly_t res;
	uint8_t i, j;

	if (a->p != b->p)
		return NULL;

	if ((res = kmalloc(sizeof(*res), GFP_KERNEL)) == NULL)
		return NULL;

	res->coef =
		kcalloc(a->deg + b->deg + 2, sizeof(*res->coef), GFP_KERNEL);
	if (res->coef == NULL)
		return NULL;

	res->deg = a->deg + b->deg;
	res->p = a->p;

	for (i = 0; i <= a->deg; i++)
		for (j = 0; j <= b->deg; j++)
			res->coef[i + j] =
				(a->coef[i] * b->coef[j] + res->coef[i + j]) %
				a->p;

	poly_normalize(res);

	return res;
}

poly_t poly_mod(c_poly_t a, c_poly_t b)
{
	poly_t res;
	uint8_t m, n, gn_inv;
	uint8_t i, j;

	if (a->p != b->p || poly_iszero(b))
		return NULL;

	res = poly_copy(a);

	if (res->deg < b->deg)
		return res;

	m = a->deg;
	n = b->deg;
	gn_inv = p_inv(b->coef[n], a->p); /* inverse for n-th b coefficient */

	/* i, j are offsets from res leading (m-th) coefficient */
	for (i = 0; i <= m - n; i++) {
		uint8_t q = (res->coef[m - i] * gn_inv) % a->p;
		for (j = i; j <= n + i; j++) {
			uint8_t subtrahend = (q * b->coef[n - j + i]) % a->p;
			res->coef[m - j] =
				p_diff(res->coef[m - j], subtrahend, a->p);
		}
	}

	poly_normalize(res);

	return res;
}

void poly_vmul(poly_t *a, c_poly_t b)
{
	poly_t old = *a;
	*a = poly_multiply(old, b);
	poly_free(old);
}

void poly_vmod(poly_t *a, c_poly_t b)
{
	poly_t old = *a;
	*a = poly_mod(old, b);
	poly_free(old);
}

uint8_t p_neg(uint8_t x, uint8_t p)
{
	return (p - x) % p;
}

uint8_t p_inv(uint8_t x, uint8_t p)
{
	return fastpow(x, p - 2) % p;
}

uint8_t p_sum(uint8_t a, uint8_t b, uint8_t p)
{
	return (a + b) % p;
}

uint8_t p_diff(uint8_t a, uint8_t b, uint8_t p)
{
	return (a + p_neg(b, p)) % p;
}

poly_t poly_get_zero(uint8_t len, uint8_t p)
{
	poly_t zero;

	if ((zero = kmalloc(sizeof(*zero), GFP_KERNEL)) == NULL)
		return NULL;

	zero->coef = kcalloc(len, sizeof(*zero->coef), GFP_KERNEL);
	zero->deg = 0;
	zero->p = p;

	return zero;
}

poly_t poly_get_identity(uint8_t len, uint8_t p)
{
	poly_t id = poly_get_zero(len, p);

	if (id == NULL)
		return NULL;

	id->coef[0] = 1;

	return id;
}

uint64_t fastpow(uint8_t x, uint8_t n)
{
	uint64_t res = 1;
	uint64_t mul = x;

	while (n > 0) {
		if (n % 2)
			res *= mul;
		mul *= mul;
		n >>= 1;
	}

	return res;
}

poly_t poly_fastpow_and_mod(c_poly_t x, uint8_t n, c_poly_t ir)
{
	poly_t res = poly_get_identity(ir->deg * 2, x->p);
	poly_t mul = poly_copy(x);

	while (n > 0) {
		if (n % 2) {
			poly_vmul(&res, mul);
			poly_vmod(&res, ir);
		}

		poly_vmul(&mul, mul);
		poly_vmod(&mul, ir);
		n >>= 1;
	}

	poly_free(mul);
	poly_normalize(res);

	return res;
}
