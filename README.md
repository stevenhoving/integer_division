Markdown convertion of blog post:
[http://codereview.blogspot.nl/2009/06/division-of-integers-by-constants.html]()

## Sunday, 21 June 2009
# Division of Integers by Constants
Nigel Jones recently wrote a blog entry that very neatly summarises an article called ['Reciprocal Multiplication, a tutorial'](http://www.cs.uiowa.edu/~jones/bcd/divide.html), by Douglas Jones.

The (Douglas) Jones article explains how to use reciprocal multiplication to perform division - for instance, if you want to divide by 10, this is the same as multiplying by 1/10. However, by doing everything in fixed point arithmetic, you can avoid the computational overhead of invoking your compiler's division routine. The result is smaller, faster code that has exactly the same accuracy as the compiler-supplied result. Anyone writing PC code may not appreciate this, but in the embedded world this sort of approach is vitally important in terms of both code size and run time.

The original article is quite long and detailed, and despite having stumbled upon it a couple of times in the past, I'd never got round to actually reading it. And so (Nigel) Jones's article came as a very welcome abstract, boiling it down as he does to a couple of simple algorithms - if you want to divide by 'x', here's what to do.

At the end of his post, (Nigel) Jones said 'If someone has too much time on their hands and would care to write a program to compute the values for all integer divisors, then I'd be happy to post the results for everyone to use.' Well, I don't know about having too much time, but I do enjoy a lunchtime programming exercise, so I sent him a text file containing the coefficients to perform integer division for all 16-bit unsigned numbers from 3 to 32768. I also asked if he would mind if I posted the source code here, in case it was of use to anyone.

He responded with an excellent suggestion - how about generating a header file containing macros with the appropriate coefficients for all divisors? Anyone wanting to use these algorithms could then simply include the header file, and call the relevant macro to perform the required division.

I've followed this suggestion, so please follow the links to check out:


* a [header](../master/integer_division_uint16.h) file containing the coefficients for unsigned 16-bit division
* a [header](../master/integer_division_uint8.h) file containing the coefficients for unsigned 8-bit division
* a command-line program generating the coefficients for unsigned 16-bit division [source](../master/integer_division_uint16.c)
* a command-line program generating the coefficients for unsigned 8-bit division [source](../master/integer_division_uint8.c)

The programs were written in Borland Builder, but are in ANSI C, and so should run on any platform with appropriate tweaks to the uintX_t typedefs. They accept floating-point arguments, and so can generate the coefficients for division by π, sqrt(2), etc. They will also report the number of errors found during an exhaustive search of all divisors, and the maximum error found.

Posted by Alan Bowens at 21:13
