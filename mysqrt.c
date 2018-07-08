#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <gmp.h>

static int progress;
static int base;
static int perfect;
static int list;
static int wrap;
static int phi;
static uint64_t bits;
static uint64_t digits;

#define	SHIFT_BITS	4	/* 2, 4, 8, 16 */
#define	SHIFT_HBITS	(SHIFT_BITS>>1)
#define	SHIFT_MASK	((1<<SHIFT_BITS)-1)

/**
 * @brief Rought estimate of the number of digits per mp_limb_t in base
 * @param libs number of limbs
 * @param base number base
 */
static int
digits_per_limb(uint64_t limbs, int base)
{
	return limbs * GMP_LIMB_BITS / base;
}

/**
 * @brief Return the decimal representation for a and b in the form "a.b"
 *
 * @param a integer part of the number to print
 * @param b fractional part of the number to print
 * @param bits number of bits in b
 * @param base base for output (e.g. 10)
 ****************************************************************/
static char *
float2str(mpz_t a, mpz_t b, uint64_t bits, int base)
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
	size = digits_per_limb(alimb, base) + digits_per_limb(blimb, base) + 1;
	buff = calloc(size + 1, sizeof(char));
	if (NULL == buff)
		return strdup("<memory error>");

	/* offs = start of the fractional part */
	offs = digits_per_limb(alimb, base) + 1;
	while (0 != mpz_cmp_ui(w, 0) && offs < size) {
		mpz_mul_ui(w, w, base);		/* multiply by base giving the next digit */
		blimb = mpz_size(b);		/* in the most significant limb */
		r = mpz_getlimbn(w, blimb) % base;
		w->_mp_d[blimb] = 0;		/* HACK: clear most significant limb again */
		/* store fractional digit to the right */
		buff[offs++] = r < 10 ? '0' + r : 'a' + r - 10;
	}

	offs = digits_per_limb(alimb, base);
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
 * The algorithm works on bytes of the number n from left to right.
 * To access the bytes, the mpz_getlimbn() function is used together with
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

	/* find the first two hex digits of the number */
	limb = mpz_getlimbn(n, --len);

	while (0 == (limb >> shift))				/* find the shift that gives a non zero byte */
		shift -= SHIFT_BITS;
	dig2 = (limb >> shift) & SHIFT_MASK;

	for (;;) {
		mpz_mul_2exp(accu, accu, SHIFT_BITS);		/* shift accu left the number of bits */
		mpz_add_ui(accu, accu, dig2);			/* put digit(s) in the least significant bits */

		/* subtract consecutive odd numbers 'step' until overflow, counting resulting digit in d */
		for (d = 0; mpz_cmp(step, accu) <= 0; d++) {
			mpz_sub(accu, accu, step);
			mpz_add_ui(step, step, 2);
		}

		mpz_mul_2exp(result, result, SHIFT_HBITS);	/* shift result left for one digit */
		mpz_add_ui(result, result, d);			/* add digit(s) into result */

		mpz_mul_2exp(step, result, SHIFT_HBITS+1);	/* next step = 2 * result * 2^hbits + 1 */
		mpz_add_ui(step, step, 1);
		if (0 == shift) {				/* if shift is 0 */
			if (--len < 0)				/* get next lower limb */
				break;
			shift = GMP_LIMB_BITS - SHIFT_BITS;	/* reset shift to limb bits - shift bits */
		} else {
			shift -= SHIFT_BITS;			/* decrease shift to use next byte */
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
		mpz_mul_2exp(accu, accu, SHIFT_BITS);		/* shift accu left the number of bits */

		/* subtract consecutive odd numbers 'step' until overflow, counting up digit in d */
		for (d = 0; mpz_cmp(step, accu) <= 0; d++) {
			mpz_sub(accu, accu, step);
			mpz_add_ui(step, step, 2);
		}

		mpz_mul_2exp(result, result, SHIFT_HBITS);	/* left shift result for one digit */
		mpz_add_ui(result, result, d);			/* add digit into result */

		mpz_mul_2exp(step, result, SHIFT_HBITS+1);	/* next step = result * << (SHIFT_HBITS + 1) */
		mpz_add_ui(step, step, 1);			/* step += 1 */
		if (progress) {
			int pct_new = (int)(10000 * calc / bits);
			if (pct_new != pct_old) {
				fprintf(stderr, "\r%d.%02d%%", pct_new/100, pct_new%100);
				pct_old = pct_new;
			}
		}
	}
	if (progress)
		fprintf(stderr, "\r%d%%   \n", 100);
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
	printf("-d nnn\tnumber of decimal digits to calculate (est.), default: %" PRIu64 "\n",
		(uint64_t)digits);
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
	phi = 0;
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
		if (!strcmp(argv[n], "-d")) {
			n++;
			digits = strtoull(argv[n], NULL, 10);
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
		if (!strcmp(argv[n], "-z")) {
			phi = 1;
			continue;
		}
		mpz_set_str(a, argv[n], 10);

		if (base < 2)
			base = 2;
		if (base > 36)
			base = 36;

		if (0 == bits) {
			/* calculate bits from digits */
			if (0 == digits)
				digits = 1000;
			bits = digits * digits_per_limb(100, base) / GMP_LIMB_BITS;
		}

		/* round to multiples of 2/4/8 bits */
		bits = (bits + SHIFT_BITS - 1) & ~(SHIFT_BITS - 1);
		gmp_printf("Calculating sqrt(%Zd) for %llu bits\n", a, bits);
		perfect = my_sqrt(integer, fraction, a, bits);
		if (perfect) {
			gmp_printf("%Zd = %Zd^2\n", a, integer);
		} else {
			if (phi) {
				/* phi = (1 + sqrt(5)) / 2 */
				mpz_add_ui(integer, integer, 1);
				mpz_div_2exp(integer, integer, 1);
				/* setting the top bit resembles the
				 * division of the odd integer part by 2
				 */
				mpz_combit(fraction, bits);
				bits++;
			}
			char* f = float2str(integer, fraction, bits, base);
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
	}
	return 0;
}
