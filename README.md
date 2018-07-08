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

An example output for `./mysqrt 2` is:
```
Calculating sqrt(2) for 10000 bits
sqrt(2) = 1.41421356237309504880168872420969807856967187530731766797379907324784621076210703885038753432764157273501384623091229702492483605585073721264412149709993583141322266592750559275579995050115278206057147010955997160597027453459686201472851741864088919860955232923048430871432145083976260362799525140798968725339654633180882964062061525835239505474575028775996172983557522033753185701135437460340849884716038689997069900481503054402779031645424782306849293691862158057846311159666871301301561856898723723528850926486124949771542183342042856860601468247207714358548741556570696776537202264854470158588016207584749226572260020855844665214583988939443709265918003113882464681570826301005948587040031864803421948972782906410450726368813137398552561173220402450912277002269411275736272804957381089675040183698683684507257993647290607629969413804756548237289971803268024744206292691248590521810044598421505911202494413417285314781058036033710773091828693147101711116839165817268894197587165821521282295184884720896
```
