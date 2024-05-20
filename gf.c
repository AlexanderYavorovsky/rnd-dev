#include <linux/slab.h>

#include "gf.h"
#include "poly.h"

/* irreducible poly x^8 + x^4 + x^3 + x^2 + 1 */
uint8_t coef2_8[] = { 1, 0, 1, 1, 1, 0, 0, 0, 1 };
struct poly ir2_8 = { .deg = 8, .coef = coef2_8, .p = 2 };
struct gf gf2_8_struct = { .p = 2, .poly = &ir2_8 };
gf_t gf2_8 = &gf2_8_struct;

/* irreducible poly x^16 + x^9 + x^8 + x^7 + x^6 + x^4 + x^3 + x^2 + 1 */
uint8_t coef2_16[] = { 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 };
struct poly ir2_16 = { .deg = 16, .coef = coef2_16, .p = 2 };
struct gf gf2_16_struct = { .p = 2, .poly = &ir2_16 };
gf_t gf2_16 = &gf2_16_struct;

/* irreducible poly x^32 + x^22 + x^2 + x + 1 */
uint8_t coef2_32[] = { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
struct poly ir2_32 = { .deg = 32, .coef = coef2_32, .p = 2 };
struct gf gf2_32_struct = { .p = 2, .poly = &ir2_32 };
gf_t gf2_32 = &gf2_32_struct;

void gf_free(gf_t ff)
{
	if (ff != NULL)
		poly_free(ff->poly);
	kfree(ff);
}

gf_t gf_init(uint8_t p, poly_t poly)
{
	gf_t ff;

	if ((ff = kmalloc(sizeof(*ff), GFP_KERNEL)) == NULL)
		return NULL;

	ff->p = p;
	ff->poly = poly_copy(poly);

	return ff;
}

int gf_isequal(gf_t ff1, gf_t ff2)
{
	return ff1->p == ff2->p && poly_isequal(ff1->poly, ff2->poly);
}

gf_elem_t gf_get_zero(gf_t ff)
{
	gf_elem_t zero;

	if ((zero = kmalloc(sizeof(*zero), GFP_KERNEL)) == NULL)
		return NULL;

	zero->poly = poly_get_zero(ff->poly->deg + 1, ff->p);
	zero->ff = ff;

	return zero;
}

gf_elem_t gf_get_identity(gf_t ff)
{
	gf_elem_t id;

	if ((id = kmalloc(sizeof(*id), GFP_KERNEL)) == NULL)
		return NULL;

	id->poly = poly_get_identity(ff->poly->deg + 1, ff->p);
	id->ff = ff;

	return id;
}

gf_elem_t gf_neg(gf_elem_t x)
{
	gf_elem_t neg;

	if ((neg = kmalloc(sizeof(*neg), GFP_KERNEL)) == NULL)
		return NULL;

	neg->ff = x->ff;
	neg->poly = poly_neg(x->poly);

	return neg;
}

gf_elem_t gf_inv(gf_elem_t x)
{
	gf_elem_t inv;
	uint64_t pow;

	if (poly_iszero(x->poly))
		return NULL;

	if ((inv = kmalloc(sizeof(*inv), GFP_KERNEL)) == NULL)
		return NULL;

	inv->ff = x->ff;
	pow = fastpow(x->ff->p, x->ff->poly->deg) - 2;
	inv->poly = poly_fastpow_and_mod(x->poly, pow, x->ff->poly);

	return inv;
}

gf_elem_t gf_sum(gf_elem_t a, gf_elem_t b)
{
	gf_elem_t res;

	if (!gf_isequal(a->ff, b->ff))
		return NULL;

	if ((res = kmalloc(sizeof(*res), GFP_KERNEL)) == NULL)
		return NULL;

	res->ff = a->ff;
	res->poly = poly_sum(a->poly, b->poly);

	return res;
}

gf_elem_t gf_subtract(gf_elem_t a, gf_elem_t b)
{
	gf_elem_t res;

	if (!gf_isequal(a->ff, b->ff))
		return NULL;

	if ((res = kmalloc(sizeof(*res), GFP_KERNEL)) == NULL)
		return NULL;

	res->ff = a->ff;
	res->poly = poly_subtract(a->poly, b->poly);

	return res;
}

gf_elem_t gf_multiply(gf_elem_t a, gf_elem_t b)
{
	gf_elem_t res;
	poly_t tmp;

	if (!gf_isequal(a->ff, b->ff))
		return NULL;

	if ((res = kmalloc(sizeof(*res), GFP_KERNEL)) == NULL)
		return NULL;

	res->ff = a->ff;
	tmp = poly_multiply(a->poly, b->poly);
	res->poly = poly_mod(tmp, a->ff->poly);

	poly_free(tmp);

	return res;
}

gf_elem_t gf_div(gf_elem_t a, gf_elem_t b)
{
	gf_elem_t res, b_inv;

	if (!gf_isequal(a->ff, b->ff) || poly_iszero(b->poly))
		return NULL;

	b_inv = gf_inv(b);
	res = gf_multiply(a, b_inv);

	gf_elem_free(b_inv);

	return res;
}

void gf_elem_free(gf_elem_t el)
{
	if (el != NULL)
		poly_free(el->poly);
	kfree(el);
}

gf_elem_t gf_elem_from_array(uint8_t *arr, uint8_t n, gf_t ff)
{
	gf_elem_t el;
	poly_t tmp_poly;

	if ((el = kmalloc(sizeof(*el), GFP_KERNEL)) == NULL)
		return NULL;

	tmp_poly = poly_init_from_array(arr, n, ff->p);
	el->poly = poly_mod(tmp_poly, ff->poly);
	el->ff = ff;

	poly_free(tmp_poly);

	return el;
}

gf_elem_t uint8_to_gf_elem(uint8_t x)
{
	gf_elem_t res = gf_get_zero(gf2_8);
	uint8_t cnt = 0;

	while (x > 0) {
		res->poly->coef[cnt++] = x % 2;
		res->poly->deg++;
		x >>= 1;
	}

	poly_normalize(res->poly);

	return res;
}

uint8_t gf_elem_to_uint8(gf_elem_t x)
{
	uint8_t res = 0;
	uint8_t mul = 1;

	for (uint8_t i = 0; i <= x->poly->deg; i++) {
		res += x->poly->coef[i] * mul;
		mul <<= 1;
	}

	return res;
}

gf_elem_t uint16_to_gf_elem(uint16_t x)
{
	gf_elem_t res = gf_get_zero(gf2_16);
	uint8_t cnt = 0;

	while (x > 0) {
		res->poly->coef[cnt++] = x % 2;
		res->poly->deg++;
		x >>= 1;
	}

	poly_normalize(res->poly);

	return res;
}

uint16_t gf_elem_to_uint16(gf_elem_t x)
{
	uint16_t res = 0;
	uint16_t mul = 1;

	for (uint8_t i = 0; i <= x->poly->deg; i++) {
		res += x->poly->coef[i] * mul;
		mul <<= 1;
	}

	return res;
}

gf_elem_t uint32_to_gf_elem(uint32_t x)
{
	gf_elem_t res = gf_get_zero(gf2_32);
	uint8_t cnt = 0;

	while (x > 0) {
		res->poly->coef[cnt++] = x % 2;
		res->poly->deg++;
		x >>= 1;
	}

	poly_normalize(res->poly);

	return res;
}

uint32_t gf_elem_to_uint32(gf_elem_t x)
{
	uint32_t res = 0;
	uint32_t mul = 1;

	for (uint8_t i = 0; i <= x->poly->deg; i++) {
		res += x->poly->coef[i] * mul;
		mul <<= 1;
	}

	return res;
}