/************************************************************************
 * Copyright 2018 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <gmp.h>

static int progress;
static int base;
static int list;
static int wrap;
static uint64_t bits;
static uint64_t digits;

#define	SHIFT_BITS	8	/* 2, 4, 8, 16 */
#define	SHIFT_HBITS	(SHIFT_BITS>>1)
#define	SHIFT_MASK	((1<<SHIFT_BITS)-1)

/**
 * @brief Rough estimate of the number of digits per mp_limb_t in base
 * @param limbs number of limbs
 * @param base number base
 */
static int
digits_for_limbs(uint64_t limbs, int base)
{
	return 1 + floor(limbs * GMP_LIMB_BITS * log(base) / log(10));
}

/**
 * @brief Return the decimal representation for a and b in the form "a.b"
 *
 * @param a integer part of the number to print
 * @param b fractional part of the number to print
 * @param bits number of bits in b
 * @param base base for output (e.g. 10)
 */
static char *
mpz2str(mpz_t a, mpz_t b, uint64_t bits, int base)
{
	mpz_t v;
	mpz_t w;
	mp_limb_t r;
	char *buff;
	uint32_t offs;
	uint32_t size;
	uint64_t alimb = mpz_size(a);
	uint64_t blimb = mpz_size(b);
	uint32_t shift;

	mpz_init(v);
	mpz_set(v, a);
	mpz_init(w);
	mpz_set(w, b);

	/* shift w left to the next full limb */
	shift = GMP_LIMB_BITS - (bits & (GMP_LIMB_BITS - 1));
	if (shift)
		mpz_mul_2exp(w, w, shift);

	/* size of the buffer required */
	size = digits_for_limbs(alimb, base) + digits_for_limbs(blimb, base) + 1;
	buff = calloc(size + 1, sizeof(char));
	if (NULL == buff)
		return strdup("<memory error>");

	/* offs = start of the fractional part */
	offs = digits_for_limbs(alimb, base) + 1;
	while (0 != mpz_cmp_ui(w, 0) && offs < size) {
		mpz_mul_ui(w, w, base);		/* multiply by base giving the next digit */
		blimb = mpz_size(b);		/* in the most significant limb */
		r = mpz_getlimbn(w, blimb) % base;
		w->_mp_d[blimb] = 0;		/* HACK: clear most significant limb again */
		/* store fractional digit to the right */
		buff[offs++] = r < 10 ? '0' + r : 'a' + r - 10;
	}
	while (offs > 1 && buff[offs - 1] == '0')
		buff[--offs] = '\0';

	offs = digits_for_limbs(alimb, base);
	buff[offs] = '.';			/* decimal point */
	while (0 != mpz_cmp_ui(v, 0) && offs > 0) {
		r = mpz_fdiv_q_ui(v, v, base);	/* divide by base and get the remainder */
		/* store next digit to the left */
		buff[--offs] = r < 10 ? '0' + r : 'a' + r - 10;
	}
	if (offs > 0)
		strcpy(buff, buff + offs);	/* left align buffer */
	mpz_clear(v);
	mpz_clear(w);
	return buff;
}

/**
 * @brief Square root function based upon the fact that n^2 = sum(1..n)[2 * n - 1]
 *
 * The algorithm works on digit pairs of the number n from left to right.
 * To access the digit pairs, the mpz_getlimbn() function is used together with
 * a current shift value decreasing from GMP_LIMB_BITS-SHIFT_BITS down to 0 by SHIFT_BITS.
 *
 * @param sqrt pointer to the mpz_t where to store the integer part of the square root
 * @param fraction pointer to the mpz_t where to store the fractional part of the square root
 * @param n number for which to find the square root
 * @param bits number of bits of fractional part to calculate
 * @return 1, if the number n is a perfect square, 0 otherwise
 */
static int
my_sqrt(mpz_t integer, mpz_t fraction, const mpz_t n, uint64_t bits)
{
	mpz_t accu;
	mpz_t result;
	mpz_t step;
	mp_limb_t limb;
	int len;
	int shift = GMP_LIMB_BITS - SHIFT_BITS;
	uint64_t calc;
	unsigned d;
	unsigned dig2;

	mpz_init(accu);
	mpz_init(result);
	mpz_init(step);
	mpz_set_ui(step, 1);

	len = mpz_size(n);					/* number of limbs in n */

	/* find the first two digits of the number */
	limb = mpz_getlimbn(n, --len);

	/* find the shift that gives a non zero digit pair */
	while (0 == (limb >> shift))
		shift -= SHIFT_BITS;
	dig2 = (limb >> shift) & SHIFT_MASK;

	for (;;) {
		/* shift accu left the number of bits */
		mpz_mul_2exp(accu, accu, SHIFT_BITS);
		/* add 2 digits in the least significant bits */
		mpz_add_ui(accu, accu, dig2);

		/* subtract consecutive odd numbers 'step' until overflow,
		 * counting up the resulting digit in d
		 */
		for (d = 0; mpz_cmp(step, accu) <= 0; d++) {
			mpz_sub(accu, accu, step);
			mpz_add_ui(step, step, 2);
		}

		/* shift result left for one digit */
		mpz_mul_2exp(result, result, SHIFT_HBITS);
		/* add the digit to the result */
		mpz_add_ui(result, result, d);

		/* next step = 2 * result * 2^hbits + 1 */
		mpz_mul_2exp(step, result, SHIFT_HBITS+1);
		mpz_add_ui(step, step, 1);
		if (0 == shift) {				/* if shift is 0 */
			if (--len < 0)				/* get next lower limb */
				break;				/* break out if no more limbs */
			/* reset shift to limb bits - shift bits */
			shift = GMP_LIMB_BITS - SHIFT_BITS;
		} else {
			/* decrease shift to use next digit pair */
			shift -= SHIFT_BITS;
		}
		limb = mpz_getlimbn(n, len);
		dig2 = (limb >> shift) & SHIFT_MASK;
	}
	mpz_init(integer);
	mpz_set(integer, result);
	if (0 == mpz_cmp_ui(accu, 0)) {				/* if the accumulator is zero */
		mpz_clear(fraction);				/* n was a perfect square */
		return 1;
	}

	int pct_old = -1;
	for (calc = 0; calc < bits; calc += SHIFT_HBITS) {
		/* shift accu left the number of bits */
		mpz_mul_2exp(accu, accu, SHIFT_BITS);

		/*
		 * subtract consecutive odd numbers 'step' until overflow,
		 * counting up the resulting digit in d
		 */
		for (d = 0; mpz_cmp(step, accu) <= 0; d++) {
			mpz_sub(accu, accu, step);
			mpz_add_ui(step, step, 2);
		}

		/* left shift result for one digit */
		mpz_mul_2exp(result, result, SHIFT_HBITS);
		/* add digit to result */
		mpz_add_ui(result, result, d);

		/* next step = result * << (SHIFT_HBITS + 1) */
		mpz_mul_2exp(step, result, SHIFT_HBITS+1);
		mpz_add_ui(step, step, 1);
		if (progress) {
			/* print progress info */
			int pct_new = (int)(10000 * calc / bits);
			if (pct_new != pct_old) {
				fprintf(stderr, "\r%d.%02d%%", pct_new/100, pct_new%100);
				pct_old = pct_new;
			}
		}
	}
	if (progress) {
		/* print final progress */
		fprintf(stderr, "\r%d%%   \n", 100);
	}
	mpz_init(fraction);
	mpz_set(fraction, result);
	mpz_clear(result);
	mpz_clear(accu);
	mpz_clear(step);
	return 0;
}

int
usage(const char* progname)
{
	printf("Usage: %s [options] number\n", progname);
	printf("Where options can be one or more of:\n");
	printf("-b nnn\tnumber of fraction bits to calculate\n");
	printf("-l\tprint result as list of digits\n");
	printf("-w\tprint result idented by 2 spaces and wrapped at 80 columns\n");
	printf("-o b\toutput result in base b (2...36), default: %d\n",
		base);
	printf("-p\tprint progress for fraction part on stderr\n");
	return 1;
}

int main(int argc, char **argv)
{
	const char *progname;
	mpz_t a;
	mpz_t integer;
	mpz_t fraction;
	int n;

	mpz_init(a);
	mpz_init(integer);
	mpz_init(fraction);

	base = 10;
	list = 0;
	wrap = 0;
	progress = 0;

	progname = strrchr(argv[0], '/');
	progname = progname ? progname + 1 : argv[0];
	if (argc < 2)
		return usage(progname);

	for (n = 1; n < argc; n++) {
		if (!strcmp(argv[n], "-b")) {
			n++;
			bits = strtoull(argv[n], NULL, 10);
			continue;
		}
		if (!strcmp(argv[n], "-h")) {
			return usage(progname);
		}
		if (!strcmp(argv[n], "-l")) {
			list = 1;
			continue;
		}
		if (!strcmp(argv[n], "-w")) {
			wrap = 1;
			continue;
		}
		if (!strcmp(argv[n], "-o")) {
			n++;
			base = strtol(argv[n], NULL, 10);
			continue;
		}
		if (!strcmp(argv[n], "-p")) {
			progress = 1;
			continue;
		}
		break;
	}
	if (n >= argc)
		return usage(progname);

	if (base < 2)
		base = 2;
	if (base > 36)
		base = 36;

	if (0 == bits) {
		/* calculate bits from digits */
		if (0 == digits)
			digits = 1000;
		bits = digits * GMP_LIMB_BITS / digits_for_limbs(1, base);
	}

	/* round to multiples of 2/4/8 bits */
	bits = (bits + SHIFT_BITS - 1) & ~(SHIFT_BITS - 1);

	mpz_set_str(a, argv[n], 10);
	if (progress) {
		gmp_printf("Calculating sqrt(%Zd) for %llu bits\n", a, bits);
	}

	if (my_sqrt(integer, fraction, a, bits)) {
		/* a is a perfect square */
		gmp_printf("%Zd = %Zd^2\n", a, integer);
	} else {
		char* f = mpz2str(integer, fraction, bits, base);
		if (list) {
			unsigned n;
			char* d;
			for (n = 1, d = f; *d; n++, d++) {
				if ('.' == *d)
					d++;
				printf("%u %c\n", n, *d);
			}
		} else if (wrap) {
			unsigned offs;
			printf("  %.78s\n", f);
			for (offs = 78; offs < strlen(f); offs += 80)
				printf("%.80s\n", f + offs);
		} else {
			gmp_printf("sqrt(%Zd) = %s\n", a, f);
		}
		free(f);
	}
	mpz_clear(a);
	mpz_clear(integer);
	mpz_clear(fraction);
	return 0;
}
