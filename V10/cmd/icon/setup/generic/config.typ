/*
 * Set up typedefs and related definitions depending on whether or not
 * ints and pointers are the same size.
 */

/* >mixed */
#if IntSize != PtrSize
#define MixedSizes
#endif IntSize != PtrSize
/* <mixed */

/* >typedef */
#ifdef MixedSizes
typedef long int word;
typedef unsigned long int uword;
#else MixedSizes
typedef int word;
typedef unsigned int uword;
#endif MixedSizes
/* <typedef */

#if PtrSize >= 24
#define LargeMem
#else PtrSize >= 24
#define SmallMem
#endif PtrSize >= 24

