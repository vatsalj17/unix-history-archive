# makefile for the math library
# Bill Nowicki March 1982
.SUFFIXES: .b .c .s

mathlib: asin.b atan.b hypot.b jn.b j0.b j1.b pow.b fabs.b \
  log.b sin.b sqrt.b tan.b tanh.b sinh.b exp.b floor.b

# how to make a .b file from a .c file
.c.b:
	cc68 -O -c $*.c

# how to make a .s file from a .c file

.c.s:
	cc68 -O -S $*.c

test:  asin.s atan.s hypot.s jn.s j0.s j1.s pow.s fabs.s \
  log.s sin.s sqrt.s tan.s tanh.s sinh.s exp.s floor.s

clean:
	rm -f *.b *.s *CKP *BAK

# old stuff
libm.a: \
asin.b atan.b \
hypot.b jn.b \
j0.b j1.b \
pow.b fabs.b \
log.b \
sin.b \
sqrt.b \
tan.b \
tanh.b \
sinh.b \
exp.b \
floor.b



