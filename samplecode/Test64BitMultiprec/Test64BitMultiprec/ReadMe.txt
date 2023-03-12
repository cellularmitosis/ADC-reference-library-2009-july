The samples in this project are intended to accompany the paper
"Special applications of 64-bit arithetic: Acceleration on the Apple G5."

addmul3asm.S, addmul4asm.S are two different 64-bit PowerPC G5 implementations of the operation A*Y+B, where A and B are arrays of 64-bit words and Y is a 64-bit integer. These routines are described in more detail in the paper.

addmulv2.c illustrates using vecLib as a straightforward alternative to the
64-bit optimized multi-precision multiplication routine described in the paper.

modmul64asm_2.S is a 64-bit routine to compute A*B (mod C) where A, B, and C 
are 64-bit quantities and a reciprocal for C has been precomputed.

