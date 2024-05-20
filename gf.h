#pragma once

#include "poly.h"

/* Galois field with irreducible polynomial `poly` and characteristic `p`,
    aka GF(p)[x]/(poly) or Fp[x]/(poly)*/
struct gf {
	poly_t poly; /* irreducible polynomial over Fp[x] */
	uint8_t p; /* characteristic of the field */
};

typedef struct gf *gf_t;

/* element of the Galois field */
struct gf_elem {
	poly_t poly;
	gf_t ff;
};

typedef struct gf_elem *gf_elem_t;

extern gf_t gf2_8;
extern gf_t gf2_16;
extern gf_t gf2_32;

/* free up memory allocated for GF */
void gf_free(gf_t ff);

/* get Fp[x]/(poly) */
gf_t gf_init(uint8_t p, poly_t poly);

/* return 1 if ff1 is the same as ff2 
  (their characteristics and irreducible polynomials match each other),
  0 otherwise */
int gf_isequal(gf_t ff1, gf_t ff2);

/* get zero element in GF */
gf_elem_t gf_get_zero(gf_t ff);

/* get identity element in GF */
gf_elem_t gf_get_identity(gf_t ff);

/* get y such that x + y = 0 in GF */
gf_elem_t gf_neg(gf_elem_t x);

/* get y such that x * y = 1 in GF */
gf_elem_t gf_inv(gf_elem_t x);

/* return a + b in GF */
gf_elem_t gf_sum(gf_elem_t a, gf_elem_t b);

/* return a - b in GF */
gf_elem_t gf_subtract(gf_elem_t a, gf_elem_t b);

/* return a * b in GF */
gf_elem_t gf_multiply(gf_elem_t a, gf_elem_t b);

/* return a / b in GF */
gf_elem_t gf_div(gf_elem_t a, gf_elem_t b);

/* free up memory allocated for GF element */
void gf_elem_free(gf_elem_t el);

/* get GF element with poly coefficients from `arr` over GF `ff` */
gf_elem_t gf_elem_from_array(uint8_t *arr, uint8_t n, gf_t ff);

/* convert uint8_t to element of GF2^8 and vice versa */
gf_elem_t uint8_to_gf_elem(uint8_t x);
uint8_t gf_elem_to_uint8(gf_elem_t x);

/* convert uint16_t to element of GF2^16 and vice versa */
gf_elem_t uint16_to_gf_elem(uint16_t x);
uint16_t gf_elem_to_uint16(gf_elem_t x);

/* convert uint32_t to element of GF2^32 and vice versa */
gf_elem_t uint32_to_gf_elem(uint32_t x);
uint32_t gf_elem_to_uint32(gf_elem_t x);