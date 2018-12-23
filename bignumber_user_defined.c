/*
 *  Multi-precision integer library
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: GPL-2.0
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

/*
 *  The following sources were referenced in the design of this Multi-precision
 *  Integer library:
 *
 *  [1] Handbook of Applied Cryptography - 1997
 *      Menezes, van Oorschot and Vanstone
 *
 *  [2] Multi-Precision Math
 *      Tom St Denis
 *      https://github.com/libtom/libtommath/blob/develop/tommath.pdf
 *
 *  [3] GNU Multi-Precision Arithmetic Library
 *      https://gmplib.org/manual/index.html
 *
 */

//TODO: чтобы избавиться от сдвигов не кратных 8 (1 байт), достаточно вычислить 8 сдвигов и вычислять с шагом 1 байт
//TODO: a mod b without DIV
//TODO: for single precession (n=1) mpi you can release faster single integer version.
//TODO: умножение можно сделать через FHT - fast hartley transform, или DFT(Zp^n)
//See: https://github.com/devoyster/IntXLib/blob/master/IntXLib/OpHelpers/FhtHelper.cs


//!!!!!! INSERT THIS FILE TO bignum.c AS !!!!!!!
//in line before mbedtls_mpi_init()
//#include "mbedtls\..\..\..\bignum_user_defined.c"
//
//and comment in bignum.h and bignum.c:
//
//  mbedtls_mpi_lsb()
//  mbedtls_clz()
//  mbedtls_mpi_get_bit()
//  mpi_write_hlp()
//  mbedtls_mpi_write_string()
//  mbedtls_mpi_mod_int() replace mbedtls_mpi_mod_uint()
//
//
// **** in bignum.h define mbedtls_t_udbl ****
//
//#if !defined(MBEDTLS_HAVE_INT32)
//	#if defined(_MSC_VER) && defined(_M_AMD64)
//	/* Always choose 64-bit when using MSC */
//	#if !defined(MBEDTLS_HAVE_INT64)
//		#define MBEDTLS_HAVE_INT64
//	#endif /* !MBEDTLS_HAVE_INT64 */
//	typedef  int64_t mbedtls_mpi_sint;
//	typedef uint64_t mbedtls_mpi_uint;
//	#if !defined(MBEDTLS_NO_UDBL_DIVISION)
//	/* mbedtls_t_udbl defined as 128-bit unsigned int */
//		typedef __uint128_t mbedtls_t_udbl;
//	#define MBEDTLS_HAVE_UDBL
//	#endif /* !MBEDTLS_NO_UDBL_DIVISION */
//
// **** in miller-rabin() replace code ****
//
//	MBEDTLS_MPI_CHK(mbedtls_mpi_fill_random(&A, X->n * ciL, f_rng, p_rng));
//	k = mbedtls_mpi_bitlen(&W);
//  A.p[A.n - 1] &= (~(mbedtls_mpi_uint)(0)) >> (((mbedtls_mpi_uint)(1) - k) & (biL - 1));
//
//	if (count++ > 30) {
//
//



#include "mbedtls/../bignum_user_defined.h"
#include <string.h>
#include <math.h> //for log-function

#ifdef biL
#undef biL
#endif
#if defined(MBEDTLS_HAVE_INT32)
#define biL   32
#define POW2L 5
#define POW2M 31
#else
#define biL   64
#define POW2L 6
#define POW2M 63
#endif
#ifdef BITS_TO_LIMBS
#undef BITS_TO_LIMBS
#endif
#define BITS_TO_LIMBS(i)   ( ((i) >> POW2L) + ( ((i) & POW2M) != 0 ) )
//#define MASK_LOWER_BITS(n) ( (~(mbedtls_mpi_uint)(0)) >> (((mbedtls_mpi_uint)(0) - (n)) & POW2M) )
#define MASK_LOWER_BITS(n) ( (~(mbedtls_mpi_uint)(0)) >> ((mbedtls_mpi_uint)(0) - (n)) )


/*
* User defined function
*/

//unsigned int _clz(mbedtls_mpi_uint x)
//{
//	unsigned int s = 0;
//	if (x == 0)return biL;
//#if defined(MBEDTLS_HAVE_INT64)
//	if (x < 0x0000000100000000ULL) { s += 32; } else { x >>= 32; }
//#endif
//	if (x < 0x00010000UL) { s += 16;x <<= 16; }
//	if (x < 0x01000000UL) { s += 8;x <<= 8; }
//	if (x < 0x10000000UL) { s += 4;x <<= 4; }
//	if (x < 0x40000000UL) { s += 2;x <<= 2; }
//	if (x < 0x80000000UL) { s += 1; }
//	return s;
//
//#if 0
//	unsigned int s = 0;
//	if (x == 0)return biL;
//#if defined(MBEDTLS_HAVE_INT64)
//	if (x < 0x0000000100000000ULL) { s = 32; } else { x >>= 32; }
//#endif
//	if (x > 0x0000FFFFUL)
//	{	
//		//priority to high 4 bits
//		if (x > 0x0FFFFFFFUL)//xxxx.0000.0000.0000
//			if (x > 0x3FFFFFFFUL)//xx00.0000.0000.0000
//				if (x > 0x7FFFFFFFUL)return 0+s;else return 1+s;
//			else //00xx.0000.0000.0000
//				if (x > 0x1FFFFFFFUL)return 2+s;else return 3+s;
//		//0000.xxxx.xxxx.xxxx
//		else if (x > 0x003FFFFFUL)//0000.xxxx.xx00.0000
//			if (x > 0x03FFFFFFUL)//0000.xx00.0000.0000
//				if (x > 0x07FFFFFFUL)return 4+s;else return 5+s;
//			//0000.00xx.xx00.0000
//			else if (x > 0x00FFFFFFUL)//0000.00xx.0000.0000
//				if (x > 0x01FFFFFFUL)return 6+s;else return 7+s;
//			else //0000.0000.xx00.0000
//				if (x > 0x007FFFFFUL)return 8+s;else return 9+s;
//		//0000.0000.00xx.xxxx
//		else if (x > 0x000FFFFFUL)//0000.0000.00xx.0000
//			if (x > 0x001FFFFFUL)return 10+s;else return 11+s;
//		//0000.0000.0000.xxxx
//		else if (x > 0x0003FFFFUL)
//			if (x > 0x0007FFFFUL)return 12+s;else return 13+s;
//		else if (x > 0x0001FFFFUL)return 14+s;else return 15+s;
//		////xxxx.xxxx.xxxx.xxxx
//		//if (x > 0x00FFFFFFUL)//xxxx.xxxx.0000.0000
//		//{
//		//	if (x > 0x0FFFFFFFUL)//xxxx.0000.0000.0000
//		//	{
//		//		if (x > 0x3FFFFFFFUL)//xx00.0000.0000.0000
//		//			if (x > 0x7FFFFFFFUL)return 0+s;else return 1+s;
//		//		else //00xx.0000.0000.0000
//		//			if (x > 0x1FFFFFFFUL)return 2+s;else return 3+s;
//		//	}
//		//	//0000.xxxx.0000.0000
//		//	else if (x > 0x03FFFFFFUL)//0000.xx00.0000.0000
//		//		if (x > 0x07FFFFFFUL)return 4+s;else return 5+s;
//		//	else //0000.00xx.0000.0000
//		//		if (x > 0x01FFFFFFUL)return 6+s;else return 7+s;
//		//}
//		////0000.0000.xxxx.xxxx
//		//else if (x > 0x000FFFFFUL)//0000.0000.xxxx.0000
//		//{
//		//	if (x > 0x003FFFFFUL)//0000.0000.xx00.0000
//		//		if (x > 0x007FFFFFUL)return 8+s;else return 9+s;
//		//	else //0000.0000.00xx.0000
//		//		if (x > 0x001FFFFFUL)return 10+s;else return 11+s;
//		//}
//		////0000.0000.0000.xxxx
//		//else if (x > 0x0003FFFFUL)//0000.0000.0000.xx00
//		//{
//		//	if (x > 0x0007FFFFUL)return 12+s;else return 13+s;
//		//}
//		////0000.0000.0000.00xx
//		//else if (x > 0x0001FFFFUL)return 14+s; else return 15+s;
//	}
//	else
//	{
//		//xxxx.xxxx.xxxx.xxxx
//		if (x > 0x00FFUL)//xxxx.xxxx.0000.0000
//		{
//			if (x > 0x0FFFUL)//xxxx.0000.0000.0000
//			{
//				if (x > 0x3FFFUL)//xx00.0000.0000.0000
//					if (x > 0x7FFFUL)return 16+s;else return 17+s;
//				else //00xx.0000.0000.0000
//					if (x > 0x1FFFUL)return 18+s;else return 19+s;
//			}
//			//0000.xxxx.0000.0000
//			else if (x > 0x03FFUL)//0000.xx00.0000.0000
//				if (x > 0x07FFUL)return 20+s;else return 21+s;
//			else //0000.00xx.0000.0000
//				if (x > 0x01FFUL)return 22+s;else return 23+s;
//		}
//		//0000.0000.xxxx.xxxx
//		else if (x > 0x000FUL)//0000.0000.xxxx.0000
//		{
//			if (x > 0x003FUL)//0000.0000.xx00.0000
//				if (x > 0x007FUL)return 24+s;else return 25+s;
//			else //0000.0000.00xx.0000
//				if (x > 0x001FUL)return 26+s;else return 27+s;
//		}
//		//0000.0000.0000.xxxx
//		else if (x > 0x0003UL)//0000.0000.0000.xx00
//		{
//			if (x > 0x0007UL)return 28+s;else return 29+s;
//		}
//		//0000.0000.0000.00xx
//		else if (x > 0x0001UL)//0000.0000.0000.00x0
//			return 30+s;
//		else if (x > 0UL)return 31+s;else return 32+s;
//	}
//#endif
//}

//z = (x + y) mod b
inline mbedtls_mpi_uint _add_uint_mod_uint(mbedtls_mpi_uint x, mbedtls_mpi_uint y, mbedtls_mpi_uint b)
{
	if (x >= b)x %= b; if (y >= b)y %= b; x = b - x;
	return y >= x ? y - x : b - (x - y);
}
//z = (x - y) mod b
inline mbedtls_mpi_uint _sub_uint_mod_uint(mbedtls_mpi_uint x, mbedtls_mpi_uint y, mbedtls_mpi_uint b)
{
	return x >= y ? (x - y) % b : b - (y - x) % b;
}
//z = (x * y) mod b
inline mbedtls_mpi_uint _mul_uint_mod_uint(mbedtls_mpi_uint x, mbedtls_mpi_uint y, mbedtls_mpi_uint b)
{
#if !defined(MBEDTLS_NO_UDBL_DIVISION)
	const mbedtls_mpi_uint h = ((mbedtls_mpi_uint)(1)) << (biL / 2);
	return x < h && y < h ? (x * y) % b : (((mbedtls_t_udbl)(x)) * y) % b;
#endif
}

//void _div_uint48to32_first(unsigned int x[2], unsigned int _y, unsigned int q[2])
//{
//	q[0] = q[1] = 0;
//	if (_y == 0) { x[0] = x[1] = 0;return; }
//	if (x[1] == 0) { q[0] = x[0] / _y;x[0] -= q[0] * _y;x[1] = 0;return; }
//
//	unsigned int c, d, d2, z, s = 0;
//	unsigned short
//		*y = (unsigned short*)(&_y);
//
//	//выравниваем y
//	if ((_y & 0x80000000UL) == 0) { s = mbedtls_clz(_y);_y <<= s; }
//
//	z = (x[1] << 16) | (x[0] >> 16);
//	c = z / y[1];z -= c * y[1];
//	z = (z << 16) | (x[0] & 0xFFFFUL);
//	d = c * y[0];
//	d2 = c > 0x10000UL && (((c & 0xFFFFUL)*y[0]) >> 16) + y[0] >= 0x10000UL ? 1 : 0;
//	if (d2 == 0 && z >= d)
//	{
//		z -= d;if (z >= _y) { c++; z -= _y; }
//	}
//	else
//	{
//		d2 = d2 != 0 && (d >= z) ? 1 : 0;
//		z = d - z;
//		if (d2)
//		{
//			if (z >= _y) { c--;z -= _y; }
//			c--;z -= _y;
//		}
//		if (z > _y) { c--;z -= _y; }
//		c--;z = _y - z;
//	}
//	if (s > 0) { c <<= s;_y >>= s;d = z / _y;z -= d * _y;c += d; }
//	q[0] = c;x[1] = 0;x[0] = z;
//}


inline unsigned int _isnan_(double x)
{
	//snan = 7FF0000000000001 - signaling NAN (raise exception from the FPU)
	//qnan = 7FF8000000000001 - quiet NAN
	// nan = 7FFFFFFFFFFFFFFF - alternative nan
	// inf = 7FF0000000000000
	//-inf = FFF0000000000000
	//exp_field = exp+1023 ([0...2047],2047=inf 11 bit), inf: exp = 1024, exp_min=-1023, exp_max=1023;
	return
		*((unsigned long long*)&x) == 0x7FF0000000000001ull || *((unsigned long long*)&x) == 0x7FF8000000000001ull || *((unsigned long long*)&x) == 0x7FFFFFFFFFFFFFFFull ||
		*((unsigned long long*)&x) == 0x7FF0000000000000ull || *((unsigned long long*)&x) == 0xFFF0000000000000ull ? 1 : 0;
}
double _to_double(const mbedtls_mpi *X)
{
	unsigned int ret;
	union { unsigned long long i; double d; } x = { 0x7FF8000000000001ull };
	//snan = 7FF0000000000001 - signaling NAN (raise exception from the FPU)
	//qnan = 7FF8000000000001 - quiet NAN
	// nan = 7FFFFFFFFFFFFFFF - alternative nan
	// inf = 7FF0000000000000
	//-inf = FFF0000000000000
	//exp_field = exp+1023 ([0...2047],2047=inf 11 bit), inf: exp = 1024, exp_min=-1023, exp_max=1023;

	mbedtls_mpi T;mbedtls_mpi_init(&T);
	MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&T, 0x7FFFFFFFL));
	if (mbedtls_mpi_cmp_abs(X, &T) <= 0) { x.d = (X->n == 0) ? 0.0 : (X->s > 0) ? (double)X->p[0] : -(double)X->p[0]; }
	else
	{
		unsigned int j = mbedtls_mpi_bitlen(X), exp = mbedtls_mpi_lsb(X);
		if (exp <= 1023 && j - exp <= 53)
		{
			//TODO: make shifting in T = uint64_t
			MBEDTLS_MPI_CHK(mbedtls_mpi_copy(&T, X));
			if (j < 53)
			{
				MBEDTLS_MPI_CHK(mbedtls_mpi_shift_l(&T, 53 - j));
			}
			else
			{
				MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&T, j - 53));
			}
			//TODO: need check 32 and 64 bits architecture (_to_double)
#if defined(MBEDTLS_HAVE_INT32)
			x.i = (((unsigned long long)(T.p[1] ^ 0x100000ul)) << 32) | ((unsigned long long)(T.p[0])) | (((unsigned long long)(j - 1 + 1023)) << 52) | ((X->s < 0) ? 0x8000000000000000ull : 0ull);
#else
			x.i = ((unsigned long long)(T.p[0] ^ 0x10000000000000ul))  | (((unsigned long long)(j - 1 + 1023)) << 52) | ((X->s < 0) ? 0x8000000000000000ull : 0ull);
#endif
		}
	}
cleanup:
	mbedtls_mpi_free(&T);
	return x.d;
}
int _from_double(mbedtls_mpi* X, double d)
{
	int ret = 0;
	union { unsigned long long i; double d; } x;
	//snan = 7FF0000000000001 - signaling NAN (raise exception from the FPU)
	//qnan = 7FF8000000000001 - quiet NAN
	// nan = 7FFFFFFFFFFFFFFF - alternative nan
	// inf = 7FF0000000000000
	//-inf = FFF0000000000000
	//exp_field = exp+1023 ([0...2047],2047=inf 11 bit), inf: exp = 1024, exp_min=-1023, exp_max=1023;

	mbedtls_mpi T;mbedtls_mpi_init(&T);
	if (X == 0 || _isnan_(d)) { ret = 1;goto cleanup; }
	x.d = d;
	if (x.i & 0x8000000000000000ull)T.s = -1;
	int exp = ((x.i >> 52) & 0x7FF) - (1023 + 52), j = 53;
	x.i &= ~0xFFF0000000000000ull;x.i |= 0x0010000000000000ull;
	if (exp < 0) { x.i = -exp < 64 ? x.i >>= -exp : 0; }
	j += exp;if (j <= 0) { j = 1; }

	//TODO: need check 32 and 64 bits architecture (_from_double)
	MBEDTLS_MPI_CHK(mbedtls_mpi_grow(&T, BITS_TO_LIMBS(j)));
	if (T.n > 0)T.p[0] = ((mbedtls_mpi_uint*)&x.i)[0];
	if (T.n > 1)T.p[1] = ((mbedtls_mpi_uint*)&x.i)[1];
	if (exp > 0) MBEDTLS_MPI_CHK(mbedtls_mpi_shift_l(&T, exp));
	mbedtls_mpi_swap(X, &T);

cleanup:
	mbedtls_mpi_free(&T);
	return ret;
}
//compute the natural logarithm of X
//for X<1 or error returns NaN
double _log(mbedtls_mpi *X)
{
	unsigned int ret, j;
	union { unsigned long long i; double d; } x = { 0x7FF8000000000001ull };
	//snan = 7FF0000000000001 - signaling NAN (raise exception from the FPU)
	//qnan = 7FF8000000000001 - quiet NAN
	// nan = 7FFFFFFFFFFFFFFF - alternative nan
	// inf = 7FF0000000000000
	//-inf = FFF0000000000000
	//exp_field = exp+1023 ([0...2047],2047=inf 11 bit), inf: exp = 1024, exp_min=-1023, exp_max=1023;

	if (mbedtls_mpi_cmp_int(X, 1) < 0) { return x.d; }

	mbedtls_mpi T;mbedtls_mpi_init(&T);
	MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&T, 0x7FFFFFFFL));
	if (mbedtls_mpi_cmp_abs(X, &T) <= 0) { x.d = (X->n == 0) ? 0.0 : (X->s > 0) ? (double)X->p[0] : -(double)X->p[0]; x.d = log(x.d); }
	else
	{
		j = mbedtls_mpi_bitlen(X);
		MBEDTLS_MPI_CHK(mbedtls_mpi_copy(&T, X));
		//TODO: need check 32 and 64 bits architecture (_log)
#if 0		
		if (j < 53)
			MBEDTLS_MPI_CHK(mbedtls_mpi_shift_l(&T, 53 - j));
		else
			MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&T, j - 53));
#if defined(MBEDTLS_HAVE_INT32)
		x.i = (((unsigned long long)(T.p[1] ^ 0x100000ul)) << 32) | ((unsigned long long)(T.p[0])) | 0x3FF0000000000000ull;
#else
		x.i = ((unsigned long long)(T.p[0] ^ 0x10000000000000ul)) | 0x3FF0000000000000ull;
#endif
		x.d = log(x.d) + ((double)(j - 1))*0.693147180559945309417232121458;
#endif
		//with roundoff
		if (j < 54)
		{
			MBEDTLS_MPI_CHK(mbedtls_mpi_shift_l(&T, 53 - j));j--;
#if defined(MBEDTLS_HAVE_INT32)
			x.i = ((unsigned long long)(T.p[1]) << 32) | ((unsigned long long)(T.p[0])) | 0x3FF0000000000000ull;
#else
			x.i = ((unsigned long long)(T.p[0])) | 0x3FF0000000000000ull;
#endif
		}
		else
		{
			MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&T, j - 54));
			unsigned int exp = j - 1 + 1023;if (exp > 2046) { j = exp - 2046;exp = 2046; } else { j = 0; }
#if defined(MBEDTLS_HAVE_INT32)
			x.i = ((unsigned long long)(T.p[1]) << 32) | ((unsigned long long)(T.p[0])); x.i += 1ull; x.i >>= 1;
#else
			x.i = ((unsigned long long)(T.p[0])); x.i += 1ull; x.i >>= 1;
#endif
			if (x.i == 0x20000000000000ull)j++;
			x.i &= ~0xFFF0000000000000ull; x.i |= ((unsigned long long)exp) << 52;
		}
		x.d = log(x.d) + ((double)j)*0.693147180559945309417232121458;
	}

cleanup:
	mbedtls_mpi_free(&T);
	return x.d;
}
















/*
 * Return the number of less significant zero-bits
 * replace to mbedtls_mpi_lsb()
 */
size_t mbedtls_mpi_lsb(const mbedtls_mpi *X)
{
	unsigned int j, c;unsigned char *p;

	if (X->n == 0)
		return 0;

	c = *((unsigned char*)X->p);
	if (c)
	{
		if (c & 1) return 0;
		if (c & 2) return 1;
		if (c & 4) return 2;
		if (c & 8) return 3;
		if (c & 16) return 4;
		if (c & 32) return 5;
		if (c & 64) return 6;
		return 7;
	}

	for (j = 0; j < X->n; j++)
		if (X->p[j] != 0)
			break;

	if (j != X->n)
	{
		c = j << POW2L; p = (unsigned char*)&X->p[j];
		for (j = 0; j < sizeof(mbedtls_mpi_uint); j++)
			if (p[j] != 0)
				break;
		c += j << 3; p = &p[j]; j = 1;
		while ((*p & j) == 0) { c++; j <<= 1; }
	}

	return c;
}

/*
 * Count leading zero bits in a given integer
 * replace to mbedtls_clz()
 */
static size_t mbedtls_clz( mbedtls_mpi_uint x )
{
	unsigned int s = 0;
	if (x == 0) return biL;
#if defined(MBEDTLS_HAVE_INT64)
	if (x < 0x100000000ULL) { s += 32; } else { x >>= 32; }
#endif
	if (x < 0x00010000UL) { s += 16;x <<= 16; }
	if (x < 0x01000000UL) { s += 8;x <<= 8; }
	if (x < 0x10000000UL) { s += 4;x <<= 4; }
	if (x < 0x40000000UL) { s += 2;x <<= 2; }
	if (x < 0x80000000UL) { s += 1; }
	return s;
}
/*
 * Get a specific bit
 */
inline int mbedtls_mpi_get_bit(const mbedtls_mpi *X, size_t pos)
{
	return (pos >> POW2L) < X->n ? (X->p[pos >> POW2L] >> pos) & 1 : 0;
	//return (pos >> POW2L) < X->n ? (X->p[pos >> POW2L] >> (pos & POW2M)) & 1 : 0;
}
/*
 * Left-shift: X <<= count
 * replace mbedtls_mpi_shift_l()
 */
int mbedtls_mpi_shift_l(mbedtls_mpi *X, size_t count)
{
	int ret;
	size_t i, v0, t1;
	mbedtls_mpi_uint r0 = 0, r1;

	i = mbedtls_mpi_bitlen(X) + count;
	i = BITS_TO_LIMBS(i);
	if (X->n < i)
		MBEDTLS_MPI_CHK(mbedtls_mpi_grow(X, i));

	v0 = count >> 3;
	t1 = count & 7;
	ret = 0;

	/*
	 * shift by bytes
	 */
	if (v0 > 0)
	{
		i = (X->n << (POW2L - 3));
		memmove((char*)X->p + v0, (char*)X->p, i - v0);
		memset((char*)X->p, 0, v0);
	}

	/*
	 * shift by count % 8
	 */
	if (t1 > 0)
	{
		for (i = v0 >> (POW2L - 3); i < X->n; i++)
		{
			r1 = X->p[i] >> (biL - t1);
			X->p[i] <<= t1;
			X->p[i] |= r0;
			r0 = r1;
		}
	}

cleanup:

	return ret;
}

/*
 * Right-shift: X >>= count
 * replace mbedtls_mpi_shift_r()
 */
int mbedtls_mpi_shift_r(mbedtls_mpi *X, size_t count)
{
	size_t i, v0, v1;
	mbedtls_mpi_uint r0 = 0, r1;

	i = count >> POW2L;
	if (i >= X->n)
		return mbedtls_mpi_lset(X, 0);

	v0 = count >> 3;
	v1 = count & 7;

	/*
	 * shift by bytes
	 */
	if (v0 > 0)
	{
		i = (X->n << (POW2L - 3));
		memmove((char*)X->p, (char*)X->p + v0, i - v0);
		memset((char*)(X->p + X->n) - v0, 0, v0);
	}

	/*
	 * shift by count % 8
	 */
	if (v1 > 0)
	{
		for (i = X->n; i > 0; i--)
		{
			r1 = X->p[i - 1] << (biL - v1);
			X->p[i - 1] >>= v1;
			X->p[i - 1] |= r0;
			r0 = r1;
		}
	}

	return 0;
}
/*
 * Export into an ASCII string
 * replace to mbedtls_mpi_write_string()
 * - radix 2...35
 */
int mbedtls_mpi_write_string(const mbedtls_mpi *X, int radix,
	char *buf, size_t buflen, size_t *olen)
{
	int ret = 0; unsigned int n; char *p;
	mbedtls_mpi T;

	if( radix < 2 || radix > 35 )
        return( MBEDTLS_ERR_MPI_BAD_INPUT_DATA );

    n = mbedtls_mpi_bitlen( X );
    if( radix >=  4 ) n >>= 1;
    if( radix >= 16 ) n >>= 1;
    /*
     * Round up the buffer length to an even value to ensure that there is
     * enough room for hexadecimal values that can be represented in an odd
     * number of digits.
     */
    n += 3 + ( ( n + 1 ) & 1 );

    if( buflen < n )
    {
        *olen = n;
        return( MBEDTLS_ERR_MPI_BUFFER_TOO_SMALL );
    }

	mbedtls_mpi_init(&T);
	p = buf;

	if (mbedtls_mpi_cmp_int(X, 0) == 0) { *p++ = '0'; }
	else if (radix == 2 || radix == 4 || radix == 16)
	{
		mbedtls_mpi_uint a; unsigned int c = 0, i, j, m = radix == 2 ? 1 : radix == 4 ? 2 : radix == 16 ? 4 : 0;

		if (X->s == -1)
			*p++ = '-';

		for (i = X->n; i > 0; i--)
			if (X->p[i - 1] != 0)break;
		a = X->p[i - 1]; for (j = 0;j < biL;j += m, a <<= m) { c = a >> (biL - m); if (c != 0)break; }
		for (j; j < biL; j += m) { c = a >> (biL - m); a <<= m;if (c < 10) *p++ = (char)(c + 0x30);/*0123456789*/ else *p++ = (char)(c + 0x37);/*ABCDEFGHIJKLMNOPQRSTUVWXYZ*/ }
		for (i--; i > 0; i--) { a = X->p[i - 1];for (j = 0; j < biL; j += m) { c = a >> (biL - m); a <<= m; if (c < 10) *p++ = (char)(c + 0x30);/*0123456789*/else *p++ = (char)(c + 0x37);/*ABCDEFGHIJKLMNOPQRSTUVWXYZ*/ } }
	}
	else
	{
		MBEDTLS_MPI_CHK(mbedtls_mpi_copy(&T, X));

		if (T.s == -1)
			T.s = 1;

		char *ptr = p + n - 1; *ptr-- = '\0';

		//k, mk=radix^k < 2^32
		const unsigned char k = ((unsigned char[]) { 0, 0, 32, 20, 16, 13, 12, 11, 10, 10, 9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 })[radix];
		const unsigned int mk = ((unsigned int[]) { 0, 0, 0, 3486784401, 0, 1220703125, 2176782336, 1977326743, 1073741824, 3486784401, 1000000000, 2357947691, 429981696, 815730721, 1475789056, 2562890625, 0, 410338673, 612220032, 893871739, 1280000000, 1801088541, 2494357888, 3404825447, 191102976, 244140625, 308915776, 387420489, 481890304, 594823321, 729000000, 887503681, 1073741824, 1291467969, 1544804416, 1838265625 })[radix];
		unsigned int a = 0, c, r, m = radix, do_work = (m & (m - 1)) == 0 ? 2 : 1;
		while (do_work)
		{
			if (do_work == 2)
			{
				a = T.p[0] & (mk - 1);
				MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&T, 32 - (32 % k)));
			}
			else
			{
				MBEDTLS_MPI_CHK(mbedtls_mpi_div_uint(&T, mk, &a));
			}
			if (mbedtls_mpi_cmp_int(&T, 0) == 0)do_work = 0;

			for (unsigned int j = 0;j < k;j++)
			{
				//TODO: что лучше: c=a%m; a=a/m;  или  r = a / m; c = a - r * m; a = r;
				r = a / m; c = a - r * m; a = r;

				if (c < 10)
					*ptr-- = (char)(c + 0x30);
				else
					*ptr-- = (char)(c + 0x37);

				if (a == 0 && do_work == 0)
					break;
			}
		}
		if (X->s == -1)
			*p++ = '-';

		if (p != ++ptr)
		{
			while (*p++ = *ptr++);
			p--;
		}
	}
	*p++ = '\0';
	*olen = p - buf;

cleanup:
	mbedtls_mpi_free(&T);
	return ret;
}

/*
 * Unsigned integer divide q = x/y, r = x - q*y
 */
int mbedtls_udbl_div_uint(const mbedtls_t_udbl *X, const mbedtls_mpi_uint y, mbedtls_t_udbl *Q, mbedtls_mpi_uint *r)
{
#if defined(MBEDTLS_HAVE_UDBL)
	/* Check for overflow */
	if (y == 0)
	{
		*Q = 0; if (r) *r = 0;
		return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
	}
	*Q = *X / y;
	if (r) *r = (mbedtls_mpi_uint)((*X ) - (*Q) * y);
	return 0;
#endif
}
/*
 * Division by unsigned int: X = X/y, r - remainder.
 */
int mbedtls_mpi_div_uint(mbedtls_mpi* X, mbedtls_mpi_uint y, mbedtls_mpi_uint *r)
{
#if defined(MBEDTLS_HAVE_UDBL)
	mbedtls_mpi_uint x[2], q[2];

	if (X == 0 || y == 0)
		return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
	if (X->n == 0 || y == 1)
	{
		if (r)*r = 0;
		return 0;
	}
	if (X->n == 1) { x[0] = X->p[0];x[1] = 0; } else { x[0] = X->p[X->n - 2];x[1] = X->p[X->n - 1]; }
	mbedtls_udbl_div_uint((mbedtls_t_udbl*)x, y, (mbedtls_t_udbl*)q, &x[0]);
	if (X->n == 1) { X->p[0] = q[0]; } else { X->p[X->n - 1] = q[1];X->p[X->n - 2] = q[0]; }
	for (int i = X->n - 3;i >= 0;i--)
	{
		x[1] = x[0];x[0] = X->p[i];
		mbedtls_udbl_div_uint((mbedtls_t_udbl*)x, y, (mbedtls_t_udbl*)q, &x[0]);
		X->p[i] = q[0];
	}
	if (r)*r = x[0];
	return 0;
#endif
}
/*
 * Modulo: r = A mod b
 * replace to mbedtls_mpi_mod_int()
 */
int mbedtls_mpi_mod_uint( mbedtls_mpi_uint *r, const mbedtls_mpi *X, mbedtls_mpi_uint b )
{
	unsigned int i; mbedtls_mpi_uint x, y = 0, cc, c0, h;

	if (X == 0 || r == 0)
		return (MBEDTLS_ERR_MPI_BAD_INPUT_DATA);
	if( b == 0 )
        return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);

    /*
     * handle trivial cases
     */
	if (b == 1) { *r = 0; return 0; }
	if (b == 2) { *r = X->p[0] & 1;return 0; }
	if (b == 4) { *r = X->p[0] & 3;return 0; }
	if (X->n == 1) { *r = X->p[0] % b;return 0; }
	/*
	 * general case
	 */
	cc = 1;c0 = ((~(mbedtls_mpi_uint)(0)) - b + 1) % b; //c0 = 2^biL mod b
	h = ((mbedtls_mpi_uint)(1) << (biL / 2));
	if (b <= h)
	{
		/*
		 * b <= 0x10000
		 */
		h <<= biL >> 2;
		for (i = 0; i < X->n; i++)
		{
			//y = (x * cc + y) mod b
			y += (X->p[i] % b)*cc;
			if (y > h) y %= b;

			//cc = (cc * c0) mod b
			cc = (cc*c0) % b;
		}
		y %= b;
	}
	else
	{
		/*
		 * b > 0x10000
		 */
		for (i = 0; i < X->n; i++)
		{
			x = X->p[i];

			//y = (x * cc + y) mod b
			x = _mul_uint_mod_uint(x, cc, b);
			y = _add_uint_mod_uint(x, y, b);

			//cc = (cc * c0) mod b
			cc = _mul_uint_mod_uint(cc, c0, b);
		}
	}

    /*
     * If A is negative, then the current y represents a negative value.
     * Flipping it to the positive side.
     */
    if( X->s < 0 && y != 0 )
        y = b - y;

    *r = y;
    return 0;
}

/*
 * Big integer sqrt: X = A^1/2
 * Warning: error occurs when E == X
 *     y = sqrt(x)
 * Newton iteration: y(k+1) = ( y(k) + x / y(k) )/2
 */
int mbedtls_mpi_sqrt_i(mbedtls_mpi *X, const mbedtls_mpi *A)
{
	int ret;
	mbedtls_mpi Y, Q, T;

	if (A->s < 0)
		return (MBEDTLS_ERR_MPI_NEGATIVE_VALUE);
	if (mbedtls_mpi_cmp_int(A, 1) <= 0)
		return mbedtls_mpi_copy(X, A);

	mbedtls_mpi_init(&Y);mbedtls_mpi_init(&Q);mbedtls_mpi_init(&T);
	//y = 1 << ( (bitlen(A) + 1)/2 )
	MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&Y, 1));
	MBEDTLS_MPI_CHK(mbedtls_mpi_shift_l(&Y, (mbedtls_mpi_bitlen(A) + 1) >> 1));
	while (1)
	{
		//y = (y + x/y) / 2
		MBEDTLS_MPI_CHK(mbedtls_mpi_div_mpi(&Q, NULL, A, &Y));
		MBEDTLS_MPI_CHK(mbedtls_mpi_add_abs(&T, &Y, &Q));
		MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&T, 1));
		if (mbedtls_mpi_cmp_mpi(&Y, &T) <= 0)
		{
			mbedtls_mpi_swap(X, &Y);break;
		}
		MBEDTLS_MPI_CHK(mbedtls_mpi_div_mpi(&Q, NULL, A, &T));
		MBEDTLS_MPI_CHK(mbedtls_mpi_add_abs(&Y, &T, &Q));
		MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&Y, 1));
		if (mbedtls_mpi_cmp_mpi(&T, &Y) <= 0)
		{
			mbedtls_mpi_swap(X, &T);break;
		}
	}

cleanup:
	mbedtls_mpi_free(&Y);mbedtls_mpi_free(&Q);mbedtls_mpi_free(&T);
	return ret;
}
/*
 * Exponentiation: X = A^E
 * Warning: error occurs when E == X
 */
int mbedtls_mpi_pow(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *E)
{
	int ret; unsigned int i, j;
	mbedtls_mpi B;

	if (E == X)
		return (MBEDTLS_ERR_MPI_BAD_INPUT_DATA);
	if (E->s < 0)
		return (MBEDTLS_ERR_MPI_NEGATIVE_VALUE);

	if (mbedtls_mpi_cmp_int(A, 1) == 0 || mbedtls_mpi_cmp_int(E, 0) == 0)
		return mbedtls_mpi_lset(X, 1);

	if (mbedtls_mpi_cmp_int(A, 0) == 0)
		return mbedtls_mpi_lset(X, 0);

	if (mbedtls_mpi_cmp_int(A, -1) == 0)
		return mbedtls_mpi_lset(X, 1 - mbedtls_mpi_get_bit(E, 0) * 2);

	mbedtls_mpi_init(&B);
	MBEDTLS_MPI_CHK(mbedtls_mpi_copy(&B, A));
	MBEDTLS_MPI_CHK(mbedtls_mpi_lset(X, 1));
	j = mbedtls_mpi_bitlen(E);
	for (i = 0; i < j; i++)
	{
		if (mbedtls_mpi_get_bit(E, i) == 1)
			MBEDTLS_MPI_CHK(mbedtls_mpi_mul_mpi(X, X, &B));
		MBEDTLS_MPI_CHK(mbedtls_mpi_mul_mpi(&B, &B, &B));
	}

cleanup:
	mbedtls_mpi_free(&B);
	return ret;
}
/*
 * Modulo: X = A mod 2^n
 */
int mbedtls_mpi_mod2n(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi_sint n)
{
	int ret = 0; unsigned int i, j;

	if (n < 0)
		return (MBEDTLS_ERR_MPI_NEGATIVE_VALUE);
	if (n == 0)
		return mbedtls_mpi_lset(X, 0);

	i = BITS_TO_LIMBS(n);
	if (A->n <= i)
	{
		if (A->s < 0)
			MBEDTLS_MPI_CHK(mbedtls_mpi_grow(X, i));
		if (X != A)
			MBEDTLS_MPI_CHK(mbedtls_mpi_copy(X, A));
	}
	else
	{
		if (X->n > i)
			memset(X->p + i, 0, (X->n - i) * sizeof(mbedtls_mpi_uint));
		else if (X->n < i)
			MBEDTLS_MPI_CHK(mbedtls_mpi_grow(X, i));
		if (X != A)
			memcpy(X->p, A->p, i * sizeof(mbedtls_mpi_uint));
		X->s = A->s;
	}
	if (X->s < 0)
	{
		for (j = 0; j < i; j++)
			if (X->p[j] != 0)
				break;
		if (j < i) X->p[j] = ((mbedtls_mpi_uint)(0)) - X->p[j];
		for (j++; j < i; j++) X->p[j] = (~(mbedtls_mpi_uint)(0)) - X->p[j];
		X->s = 1;
	}
	if (X->n >= i)
		X->p[i - 1] &= MASK_LOWER_BITS(n);

cleanup:
	return ret;
}

/*
 * Helper for mbedtls_mpi multiplication
 * (only declaration)
 */
static
#if defined(__APPLE__) && defined(__arm__)
	__attribute__((noinline))
#endif
void mpi_mul_hlp(size_t i, mbedtls_mpi_uint *s, mbedtls_mpi_uint *d, mbedtls_mpi_uint b);

/*
 * Baseline multiplication: X = A*B mod 2^n (little modified version of original function)
 */
int mbedtls_mpi_mulmod2n(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *B, const mbedtls_mpi_sint n)
{
	int ret; unsigned int i, j, k;
	mbedtls_mpi TA, TB;

	if (n < 0)
		return (MBEDTLS_ERR_MPI_NEGATIVE_VALUE);
	if (n == 0)
		return mbedtls_mpi_lset(X, 0);

	mbedtls_mpi_init(&TA); mbedtls_mpi_init(&TB);
	MBEDTLS_MPI_CHK(mbedtls_mpi_mod2n(&TA, A, n));
	MBEDTLS_MPI_CHK(mbedtls_mpi_mod2n(&TB, B, n));

	i = TA.n; j = TB.n; k = BITS_TO_LIMBS(n);

	MBEDTLS_MPI_CHK(mbedtls_mpi_grow(X, k + 1));
	MBEDTLS_MPI_CHK(mbedtls_mpi_lset(X, 0));

	for (j = 0; j < TB.n; j++)
	{
		mpi_mul_hlp(j > k - i ? k - j : i, TA.p, X->p + j, TB.p[j]);
		X->p[k] = 0;
	}

	X->s = TA.s * TB.s;
	if (X->s > 0)
		X->p[k - 1] &= MASK_LOWER_BITS(n);
	else
		MBEDTLS_MPI_CHK(mbedtls_mpi_mod2n(X, X, n));

cleanup:
	mbedtls_mpi_free(&TA); mbedtls_mpi_free(&TB);
	return ret;
}
/*
 * Modular inverse: A*X = C (mod 2^n)
 *    if (C mod 2^n) == 0 then return X=0;
 *    A = A/gcd(A,C,2^n); C = C/gcd(A,C,2^n);
 *    if (A mod 2)   == 0 then return DIVISION_BY_ZERO;
 *    X = C/A mod 2^n;
 * Formula:
 * do { x = (2 - A*x)*x mod 2^s; s=s+s; } while(s < n);
 * X = x*C mod 2^n;
 */
int mbedtls_mpi_invmod2n(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi_sint n, const mbedtls_mpi *C)
{
	int ret; unsigned int k, s, na; mbedtls_mpi_uint x;
	mbedtls_mpi T, TA, TC, TT;

	if (n < 0)
		return (MBEDTLS_ERR_MPI_NEGATIVE_VALUE);

	s = mbedtls_mpi_lsb(A);
	k = 0;
	//if (C mod 2^n) == 0 then return X=0;
	if (C != NULL)
	{
		k = mbedtls_mpi_lsb(C);
		if (mbedtls_mpi_cmp_int(C, 0) == 0 || k >= (unsigned int)n)
			return mbedtls_mpi_lset(X, 0);
	}
	//if (A / gcd(A,C,2^n) mod 2) == 0 then return DIVISION_BY_ZERO;
	if (mbedtls_mpi_cmp_int(A, 0) == 0 || s > k)
		return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);

	mbedtls_mpi_init(&T);mbedtls_mpi_init(&TA);mbedtls_mpi_init(&TC);
	MBEDTLS_MPI_CHK(mbedtls_mpi_mod2n(&TA, A, n));
	if (C != NULL)
		MBEDTLS_MPI_CHK(mbedtls_mpi_mod2n(&TC, C, n));
	if (s > 0)
	{
		MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&TA, s));
		MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&TC, s));
	}

	//if (A mod 2^n) == 1 then return (C mod 2^n);
	if (mbedtls_mpi_cmp_int(&TA, 1) == 0)
	{
		if (C == NULL)
			mbedtls_mpi_swap(&TA, X);
		else
			mbedtls_mpi_swap(&TC, X);
		goto cleanup;
	}

	s = 2; na = n < biL ? n : biL; x = TA.p[0] & 3;
	while (s < na)
	{
		s <<= 1;x *= 2 - TA.p[0] * x; x &= MASK_LOWER_BITS(s);
	}
	MBEDTLS_MPI_CHK(mbedtls_mpi_lset(X, 0));X->p[0] = x;
	if (n > biL)
	{
		s = 1; na = TA.n; k = BITS_TO_LIMBS(n); TT.p = TA.p;TT.s = TA.s;
		while (s < k)
		{
			s <<= 1; if (s > k)s = k; TT.n = s < na ? s : na;
			MBEDTLS_MPI_CHK(mbedtls_mpi_mulmod2n(&T, X, &TT, s << POW2L));T.s = -T.s;
			MBEDTLS_MPI_CHK(mbedtls_mpi_add_int(&T, &T, 2));
			MBEDTLS_MPI_CHK(mbedtls_mpi_mulmod2n(X, X, &T, s << POW2L));
		}
	}
	if (C == NULL)
		MBEDTLS_MPI_CHK(mbedtls_mpi_mod2n(X, X, n));
	else
		MBEDTLS_MPI_CHK(mbedtls_mpi_mulmod2n(X, X, &TC, n));

cleanup:
	mbedtls_mpi_free(&T); mbedtls_mpi_free(&TA);mbedtls_mpi_free(&TC);
	return ret;
}

/*
 * Exponentiation: X = A^E mod 2^n
 */
int mbedtls_mpi_expmod2n(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *E, const mbedtls_mpi_sint n)
{
	int ret; unsigned int i, j;
	mbedtls_mpi T, B;

	if (n < 0)
		return (MBEDTLS_ERR_MPI_NEGATIVE_VALUE);
	if (n == 0)
		return mbedtls_mpi_lset(X, 0);

	//here:
	//  A^0 = 1
	//  0^0 = 1
	//  0^e = 0,   e > 0
	//  0^e = 1/0, e < 0

	//if A == 1 || (E mod 2^(n-1)) == 0 then return X=1; here Phi(2^n) = 2^(n-1)
	//i = mbedtls_mpi_lsb(E);
	//if (mbedtls_mpi_cmp_int(A, 1) == 0 || mbedtls_mpi_cmp_int(E, 0) == 0 || i >= (unsigned int)(n - 1))
	//	return mbedtls_mpi_lset(X, 1);

	//if A == 1 || E == 0 then return X=1;
	if (mbedtls_mpi_cmp_int(A, 1) == 0 || mbedtls_mpi_cmp_int(E, 0) == 0)
		return mbedtls_mpi_lset(X, 1);

	//if E < 0 && (A mod 2) == 0 then return DIVISION_BY_ZERO;
	if (E->s < 0 && mbedtls_mpi_get_bit(A, 0) == 0)
		return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);

	//if (A mod 2^n) == 0 || lsb(A) * E >= n then return X=0;
	j = mbedtls_mpi_lsb(A);
	if (mbedtls_mpi_cmp_int(A, 0) == 0 || j >= (unsigned int)(n) || j != 0 && mbedtls_mpi_cmp_int(E, (n + j - 1) / j) >= 0)
		return mbedtls_mpi_lset(X, 0);

	//if A == -1 then return X=(-1)^(E mod 2);
	if (mbedtls_mpi_cmp_int(A, -1) == 0)
		return
			mbedtls_mpi_lset(X, 1 - mbedtls_mpi_get_bit(E, 0) * 2) == 0 &&
			mbedtls_mpi_mod2n(X, X, n) == 0 ? 0 : (MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);
	
	mbedtls_mpi_init(&T);mbedtls_mpi_init(&B);
	MBEDTLS_MPI_CHK(mbedtls_mpi_mod2n(&B, A, n));

	//TODO: выбрать вариант для E<0 : 1) E = E mod 2^(n-1); или  2) A=A^-1 mod 2^n; E=-E;
	MBEDTLS_MPI_CHK(mbedtls_mpi_mod2n(&T, E, n - 1)); //T = E mod 2^(n-1)
	if (E->s < 0 && mbedtls_mpi_bitlen(E) + biL - mbedtls_clz(n) < mbedtls_mpi_bitlen(&T))
	{
		MBEDTLS_MPI_CHK(mbedtls_mpi_invmod2n(&B, &B, n, NULL)); //B = A^-1 mod 2^n
		MBEDTLS_MPI_CHK(mbedtls_mpi_copy(&T, E)); T.s = -T.s;   //T = -E
	}

	MBEDTLS_MPI_CHK(mbedtls_mpi_lset(X, 1));
	j = mbedtls_mpi_bitlen(&T);
	for (i = 0; i < j; i++)
	{
		if (mbedtls_mpi_get_bit(&T, i) == 1)
			MBEDTLS_MPI_CHK(mbedtls_mpi_mulmod2n(X, X, &B, n));
		MBEDTLS_MPI_CHK(mbedtls_mpi_mulmod2n(&B, &B, &B, n));
	}

cleanup:
	mbedtls_mpi_free(&T);mbedtls_mpi_free(&B);
	return ret;
}

/*
 * Full exponentiation: X = A^E mod N
 */
int mbedtls_mpi_expmod_full(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *E, const mbedtls_mpi *N)
{
	int ret; unsigned int n;
	mbedtls_mpi T, B, AA, BB, TT;

	if (N->s < 0)
		return (MBEDTLS_ERR_MPI_NEGATIVE_VALUE);
	if (mbedtls_mpi_cmp_int(N, 0) == 0)
		return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
	if (mbedtls_mpi_cmp_int(N, 1) == 0)
		return mbedtls_mpi_lset(X, 0);
	
	//here:
	//  A^0 = 1
	//  0^0 = 1
	//  0^e = 0,   e > 0
	//  0^e = 1/0, e < 0

	//if A == 1 || E == 0 return X = 1;
	if (mbedtls_mpi_cmp_int(A, 1) == 0 || mbedtls_mpi_cmp_int(E, 0) == 0)
		return mbedtls_mpi_lset(X, 1);

	//if A == 0 && E > 0 return X = 0;
	if (mbedtls_mpi_cmp_int(A, 0) == 0)
		if (E->s < 0)
			return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
		else
			return mbedtls_mpi_lset(X, 0);

	mbedtls_mpi_init(&T);mbedtls_mpi_init(&B);mbedtls_mpi_init(&AA);mbedtls_mpi_init(&BB);
	// if N is odd then return X = A^E mod N;
	if (mbedtls_mpi_get_bit(N, 0) == 1)
	{
		if (E->s > 0)
			MBEDTLS_MPI_CHK(mbedtls_mpi_exp_mod(X, A, E, N, NULL));
		else
		{	// when (E < 0) {  X = A^-1 mod N;   X = X^-E mod N;  }
			TT.p = E->p;TT.n = E->n;TT.s = 1;
			//MBEDTLS_MPI_CHK(mbedtls_mpi_inv_mod(X, A, N));
			MBEDTLS_MPI_CHK(mbedtls_mpi_invmod_hlp(X, A, N, NULL));
			MBEDTLS_MPI_CHK(mbedtls_mpi_exp_mod(X, X, &TT, N, NULL));
		}
	}
	else
	{
		// Evaluate: A^E mod B*2^n = X1*B + X0
		// 1) BB = (B^-1) mod 2^n;
		// 2)  T = A^E mod B;
		// 3)  X = A^E mod 2^n; X = X - T mod 2^n; X = X*BB mod 2^n;
		// Result: X = X*B + T
		n = mbedtls_mpi_lsb(N);
		if (X == A) { mbedtls_mpi_swap(&AA, (mbedtls_mpi*)A);A = &AA; }
		MBEDTLS_MPI_CHK(mbedtls_mpi_expmod2n(X, A, E, n));
		if (mbedtls_mpi_bitlen(N) > n + 1)
		{
			MBEDTLS_MPI_CHK(mbedtls_mpi_copy(&B, N));
			MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&B, n));
			MBEDTLS_MPI_CHK(mbedtls_mpi_invmod2n(&BB, &B, n, NULL));
			if (E->s > 0)
				MBEDTLS_MPI_CHK(mbedtls_mpi_exp_mod(&T, A, E, &B, NULL));
			else
			{	//if (E < 0) {  X = A^-1 mod N;   X = X^-E mod N;  }
				TT.p = E->p;TT.n = E->n;TT.s = 1;
				MBEDTLS_MPI_CHK(mbedtls_mpi_invmod_hlp(&T, A, &B, NULL));
				MBEDTLS_MPI_CHK(mbedtls_mpi_exp_mod(&T, &T, &TT, &B, NULL));
			}
			MBEDTLS_MPI_CHK(mbedtls_mpi_sub_mpi(X, X, &T));
			MBEDTLS_MPI_CHK(mbedtls_mpi_mulmod2n(X, X, &BB, n));
			MBEDTLS_MPI_CHK(mbedtls_mpi_mul_mpi(X, X, &B));
			MBEDTLS_MPI_CHK(mbedtls_mpi_add_mpi(X, X, &T));
		}
	}

cleanup:
	mbedtls_mpi_free(&T);mbedtls_mpi_free(&B);mbedtls_mpi_free(&AA);mbedtls_mpi_free(&BB);
	return ret;
}

/*
 * Euclid solver by division
 *  X = A^-1 mod N
 */
//int mbedtls_mpi_inv_mod_DIV(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *N)
//{
//	unsigned int ret = 0;
//	mbedtls_mpi T, Q, R1, R2, U1, U2;
//
//	if (mbedtls_mpi_cmp_int(N, 0) == 0)
//		return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
//	if (mbedtls_mpi_cmp_int(N, 0) < 0)
//		return (MBEDTLS_ERR_MPI_NEGATIVE_VALUE);
//	if (mbedtls_mpi_bitlen(A) <= 1)
//	{
//		if (mbedtls_mpi_cmp_int(A, 0) == 0)
//			return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
//		if (mbedtls_mpi_cmp_int(A, 1) == 0)
//			return mbedtls_mpi_lset(X, 1);
//		if (mbedtls_mpi_cmp_int(A, -1) == 0)
//			return mbedtls_mpi_add_mpi(X, A, N);
//	}
//
//	mbedtls_mpi_init(&T);mbedtls_mpi_init(&Q);mbedtls_mpi_init(&R1);mbedtls_mpi_init(&R2);mbedtls_mpi_init(&U1);mbedtls_mpi_init(&U2);
//	//A mod N  = R1
//	MBEDTLS_MPI_CHK(mbedtls_mpi_div_mpi(&Q, &R1, A, N));Q.s = 1;R1.s = 1;
//	if (mbedtls_mpi_cmp_int(&R1, 0) == 0)//error on gcd(A,N) > 1
//		MBEDTLS_MPI_CHK(MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
//	//N mod R1 = R2
//	MBEDTLS_MPI_CHK(mbedtls_mpi_div_mpi(&U2, &R2, N, &R1));mbedtls_mpi_lset(&U1, 1);U2.s = -1;
//	//U1 = 1, U2 = -Q
//	if (mbedtls_mpi_cmp_int(&R2, 0) != 0) while (1)
//	{
//		mbedtls_mpi_swap(&U1, &U2);
//		mbedtls_mpi_swap(&R1, &R2);
//		MBEDTLS_MPI_CHK(mbedtls_mpi_div_mpi(&Q, &R2, &R2, &R1));
//		if (mbedtls_mpi_cmp_int(&R2, 0) == 0)
//			break;
//		MBEDTLS_MPI_CHK(mbedtls_mpi_mul_mpi(&T, &Q, &U1));
//		MBEDTLS_MPI_CHK(mbedtls_mpi_sub_mpi(&U2, &U2, &T));
//	}
//	if (mbedtls_mpi_cmp_int(&R1, 1) > 0)//error on gcd(A,N) > 1
//		MBEDTLS_MPI_CHK(MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
//
//	U1.s *= A->s;
//	if (mbedtls_mpi_cmp_int(&U1, 0) < 0)
//		MBEDTLS_MPI_CHK(mbedtls_mpi_add_mpi(&U1, &U1, N));
//	mbedtls_mpi_swap(&U1, X);
//
//cleanup:
//	mbedtls_mpi_free(&T);mbedtls_mpi_free(&Q);mbedtls_mpi_free(&R1);mbedtls_mpi_free(&R2);mbedtls_mpi_free(&U1);mbedtls_mpi_free(&U2);
//	return ret;
//}

/*
 *  Helper function of binary algorithm for inversion, odd modulus, no argument checking.
 *      A*X = C mod B,   B - odd.
 *  If C is undefined, then set C = 1.
 *  [Algorithm 2.22. Guide to Elliptic Curve Cryptography. - D.Hankerson, A.Menezes, S.Vanstone]
 */
int mbedtls_mpi_invmod_hlp(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *B, const mbedtls_mpi *C)
{
	int ret; unsigned int s;
	mbedtls_mpi AA, BB, U, V;

	//if (B->s < 0)
	//	return (MBEDTLS_ERR_MPI_NEGATIVE_VALUE);
	//if (mbedtls_mpi_get_bit(B, 0) == 0)
	//	return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);

	mbedtls_mpi_init(&AA);mbedtls_mpi_init(&BB);mbedtls_mpi_init(&U);mbedtls_mpi_init(&V);
	MBEDTLS_MPI_CHK(mbedtls_mpi_mod_mpi(&AA, A, B));//TODO: A mod B сделать без деления
	if (C == NULL)
		MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&U, 1));
	else
	{
		//if (C mod B) == 0 then return X=0;
		MBEDTLS_MPI_CHK(mbedtls_mpi_mod_mpi(&U, C, B));
		if (mbedtls_mpi_cmp_int(&U, 0) == 0) { mbedtls_mpi_swap(&U, X);goto cleanup; }
		//if gcd(A,C) > 1 then A = A/gcd(A,C) and C = C/gcd(A,C)
		if (mbedtls_mpi_cmp_int(&AA, 1) > 0)
		{
			MBEDTLS_MPI_CHK(mbedtls_mpi_gcd(&V, &AA, &U));
			if (mbedtls_mpi_cmp_int(&V, 1) != 0)
			{
				MBEDTLS_MPI_CHK(mbedtls_mpi_div_mpi(&AA, NULL, &AA, &V));
				MBEDTLS_MPI_CHK(mbedtls_mpi_div_mpi(&U, NULL, &U, &V));
			}
		}
	}
	MBEDTLS_MPI_CHK(mbedtls_mpi_copy(&BB, B));
	MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&V, 0));
	while (mbedtls_mpi_cmp_int(&AA, 1) > 0 && mbedtls_mpi_cmp_int(&BB, 1) > 0)
	{
		s = mbedtls_mpi_lsb(&AA);
		MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&AA, s));
		while (s > 0)
		{
			if (mbedtls_mpi_get_bit(&U, 0) == 1)
				MBEDTLS_MPI_CHK(mbedtls_mpi_add_mpi(&U, &U, B));
			MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&U, 1));
			s--;
		}
		s = mbedtls_mpi_lsb(&BB);
		MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&BB, s));
		while (s > 0)
		{
			if (mbedtls_mpi_get_bit(&V, 0) == 1)
				MBEDTLS_MPI_CHK(mbedtls_mpi_add_mpi(&V, &V, B));
			MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&V, 1));
			s--;
		}
		if (mbedtls_mpi_cmp_mpi(&AA, &BB) >= 0)
		{
			MBEDTLS_MPI_CHK(mbedtls_mpi_sub_mpi(&AA, &AA, &BB));
			MBEDTLS_MPI_CHK(mbedtls_mpi_sub_mpi(&U, &U, &V));
		}
		else
		{
			MBEDTLS_MPI_CHK(mbedtls_mpi_sub_mpi(&BB, &BB, &AA));
			MBEDTLS_MPI_CHK(mbedtls_mpi_sub_mpi(&V, &V, &U));
		}
	}
	if (mbedtls_mpi_cmp_int(&AA, 0) == 0 || mbedtls_mpi_cmp_int(&BB, 0) == 0)
		MBEDTLS_MPI_CHK(MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
	if (mbedtls_mpi_cmp_int(&AA, 1) != 0)
		mbedtls_mpi_swap(&V, &U);
	if (mbedtls_mpi_cmp_int(&U, 0) < 0)
		MBEDTLS_MPI_CHK(mbedtls_mpi_sub_abs(X, B, &U));
	else
		mbedtls_mpi_swap(X, &U);

cleanup:
	mbedtls_mpi_free(&AA);mbedtls_mpi_free(&BB);mbedtls_mpi_free(&U);mbedtls_mpi_free(&V);
	return ret;
}

/*
 *  A*X = C mod N,   N > 0, if C == NULL, then C = 1
 */
int mbedtls_mpi_invmod_full(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *N, const mbedtls_mpi *C)
{
	int ret; unsigned int n;
	mbedtls_mpi T, B, AA, CC;
	
	if (N->s < 0)
		return (MBEDTLS_ERR_MPI_NEGATIVE_VALUE);
	if (mbedtls_mpi_cmp_int(N, 0) == 0)
		return (MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
	if (mbedtls_mpi_get_bit(N, 0) == 1)
		return mbedtls_mpi_invmod_hlp(X, A, N, C);

	mbedtls_mpi_init(&T);mbedtls_mpi_init(&B);mbedtls_mpi_init(&AA);mbedtls_mpi_init(&CC);
	n = mbedtls_mpi_lsb(N);
	if (n == 1)
	{	//Case n == 1.
		//Solve: A*X mod B*2 = C,  X=X1*B+X0
		//X0 = invmod_hlp(A,B,C);
		//X1 = (C - A*X0) mod 2;
		//if (A mod 2) == 0 && (C mod 2) == 1 then DIVISION_BY_ZERO
		if (mbedtls_mpi_get_bit(A, 0) == 0 && (C == NULL || mbedtls_mpi_get_bit(C, 0) == 1))
			MBEDTLS_MPI_CHK(MBEDTLS_ERR_MPI_DIVISION_BY_ZERO);
		MBEDTLS_MPI_CHK(mbedtls_mpi_copy(&B, N));
		MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&B, n));
		MBEDTLS_MPI_CHK(mbedtls_mpi_invmod_hlp(X, A, &B, C));//X0
		n = 1 - mbedtls_mpi_get_bit(A, 0)*mbedtls_mpi_get_bit(X, 0);
		if (C != NULL && mbedtls_mpi_get_bit(C, 0) == 0)
			n ^= 1;
		if (n)
			MBEDTLS_MPI_CHK(mbedtls_mpi_add_mpi(X, X, &B));
		goto cleanup;
	}

	//Case n > 1.
	// Solve: A*(X1*2^n + X0) mod B*2^n = C
	// 1) A*X0 mod 2^n = C; ==> X0 = invmod2n(A,n,C);
	// 2) T = (C - A*X0)>>n;
	// 3) A*X1 mod B = T;   ==> X1 = invmod(A,B,T);
	// Result: X = (X1<<n) + X0
	if (X == A) { mbedtls_mpi_swap(X, &AA);A = &AA; }
	if (C == NULL) MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&CC, 1));
	MBEDTLS_MPI_CHK(mbedtls_mpi_invmod2n(X, A, n, C));//X0
	if (mbedtls_mpi_bitlen(N) > n + 1)
	{
		MBEDTLS_MPI_CHK(mbedtls_mpi_copy(&B, N));
		MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&B, n));
		MBEDTLS_MPI_CHK(mbedtls_mpi_mul_mpi(&T, A, X));
		MBEDTLS_MPI_CHK(mbedtls_mpi_sub_mpi(&T, (C == NULL) ? &CC : C, &T));
		MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&T, n));//T
		MBEDTLS_MPI_CHK(mbedtls_mpi_invmod_hlp(&T, A, &B, &T));
		MBEDTLS_MPI_CHK(mbedtls_mpi_shift_l(&T, n));
		MBEDTLS_MPI_CHK(mbedtls_mpi_add_mpi(X, X, &T));
	}

cleanup:
	mbedtls_mpi_free(&T);mbedtls_mpi_free(&B);mbedtls_mpi_free(&AA);mbedtls_mpi_free(&CC);
	return ret;
}
