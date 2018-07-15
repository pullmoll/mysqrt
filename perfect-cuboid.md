# The perfect cuboid

Rectangular cuboids where the side lengths *a*, *b*, and *c* as well as lengths of the face
diagonals *d*, *e*, and *f* are all integers are also called „Euler bricks“.
The face diagonals are:

 <tt>d	= √(a<sup>2</sup> + b<sup>2</sup>)</tt><br/>
 <tt>e	= √(a<sup>2</sup> + c<sup>2</sup>)</tt><br/>
 <tt>f	= √(b<sup>2</sup> + c<sup>2</sup>)</tt><br/>

A primitive Pythagorean Triple (*pPT*) is a triplet of the sides lengths of a
rectangular triangle with one odd length leg, one even length leg and an odd
length hypotenuse, where all lengths are mutually prime.
The face triangles of any rectangular cuboid are general Pythagorean Triples (*PT*),
which means they are each multiples of a *pPT* and a factor *x*, *y*, and *z* in a unique way:

 <tt>d = x · d<sub>x</sub> = x · √(a<sub>x</sub><sup>2</sup> + b<sub>x</sub><sup>2</sup>)</tt><br/>
 <tt>e = y · e<sub>y</sub> = y · √(a<sub>y</sub><sup>2</sup> + c<sub>y</sub><sup>2</sup>)</tt><br/>
 <tt>f = z · f<sub>z</sub> = z · √(b<sub>z</sub><sup>2</sup> + c<sub>z</sub><sup>2</sup>)</tt><br/>

In a perfect cuboid also the space diagonal *g* would be an integer.
The square of the length of the space diagonal *g* is described by the equation:

 <tt>g<sup>2</sup> = a<sup>2</sup> + b<sup>2</sup> + c<sup>2</sup></tt>

There are three more, equivalent equations which describe the length of *g*:

 <tt>g<sup>2</sup> = d<sup>2</sup> + c<sup>2</sup></tt></br>
  <tt>	= (x · d<sub>x</sub>)<sup>2</sup> + c<sup>2</sup></tt></br>
  <tt>	= x<sup>2</sup> · d<sub>x</sub><sup>2</sup> + c<sup>2</sup></tt></br>
  <tt>	= x<sup>2</sup> · (a<sub>x</sub><sup>2</sup> + b<sub>x</sub><sup>2</sup>) + c<sup>2</sup></tt>

 <tt>g<sup>2</sup> = e<sup>2</sup> + b<sup>2</sup></tt></br>
  <tt>	= (y · e<sub>y</sub>)<sup>2</sup> + b<sup>2</sup></tt></br>
  <tt>	= y<sup>2</sup> · e<sub>y</sub><sup>2</sup> + b<sup>2</sup></tt></br>
  <tt>	= y<sup>2</sup> · (a<sub>y</sub><sup>2</sup> + c<sub>y</sub><sup>2</sup>) + b<sup>2</sup></tt>

 <tt>g<sup>2</sup> = f<sup>2</sup> + a<sup>2</sup></tt></br>
  <tt>  = (z · f<sub>z</sub>)<sup>2</sup> + a<sup>2</sup></tt></br>
  <tt>  = z<sup>2</sup> · f<sub>z</sub><sup>2</sup> + a<sup>2</sup></tt></br>
  <tt>  = z<sup>2</sup> · (b<sub>z</sub><sup>2</sup> + c<sub>z</sub><sup>2</sup>) + a<sup>2</sup></tt>

Until today no perfect cuboid was found by searches run on computers and it is unknown,
whether there is a proof supporting or neglecting the existence of such a perfect cuboid.

Now since *d*, *e*, and *f* are all multiples of the hypotenuse of a *pPT*,
they are all multiples of an odd length. On the other hand all of *a*, *b*, and *c* are
multiples of the legs (cathetus) of two of the three *pPT*s and as such they are multiples
of either the even or the odd leg of those *pPT*s:

 <tt>a = x · a<sub>x</sub> = y · a<sub>y</sub></tt><br/>
 <tt>b = x · b<sub>x</sub> = z · b<sub>z</sub></tt><br/>
 <tt>c = y · c<sub>y</sub> = z · c<sub>z</sub></tt><br/>

Since every pPT has exactly one odd length leg, one even length leg, and an odd length hypotenuse,
*g* cannot at the same time satisfy all three of the equations involving one face diagonal
*d*, *e*, or *f* for one leg and the opposite side length *c*, *b*, or *a* for the other leg of a *PT*.

Exactly one of *a*, *b*, or *c* is in two ways a multiple of the odd length leg of two *pPT*s.
Because no *pPT* has two odd legs, the *PT* giving *g* does not exist.

Jürgen Buchmüller <pullmoll@t-online.de>
