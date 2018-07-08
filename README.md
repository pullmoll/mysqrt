### Calculating the square root of a number - digit by digit


#### The basic algorithm.
The algorithm used here is not fast, but accurate for each digit which is calculated.

It divides the number into groups of *two digits*, where the term digits depends on
the number of bits per step. It can be 2 (2 * 1-bit digits), 4 (2 * 2-bit digits),
8 (2 4-bit digits) etc.

The name used in this source is SHIFT_BITS.
The name used for the bits in one digit is SHIFT_HBITS.

Taking the leftmost digit pair into an accumulator (accu) it subtracts the
sequence of odd numbers, starting at 1, until the result underflows.
The count of successful subtractions before the underflow occurs is the first
digit of the result.

The next two digits of the number are then appended to the remainder of this sequence
of subtractions.

The result so far is shifted left by (SHIFT_HBITS + 1) and 1 is added.
This then is the start value (step) for subtracting consecutive odd numbers
from the accumulator again.

This continue as long as more bits are available in the input number.

If there are no more bits, then if the accumulator is zero at this point,
the number was a perfect square.

Otherwise the shifting and subtracting continues for a specified number of
requested result bits. Instead of appending more digit pairs of the number to
the accumulator, accu is just shifted left by two digits and nothing (or "00")
is added.
