/**
 * \file bignum_user_defined.h
 *
 * \brief Multi-precision integer library
 */
/*
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

#ifdef __cplusplus
extern "C" {
#endif
/**
 * User defined function
 */
inline mbedtls_mpi_uint _add_uint_mod_uint(mbedtls_mpi_uint x, mbedtls_mpi_uint y, mbedtls_mpi_uint b);
inline mbedtls_mpi_uint _sub_uint_mod_uint(mbedtls_mpi_uint x, mbedtls_mpi_uint y, mbedtls_mpi_uint b);
inline mbedtls_mpi_uint _mul_uint_mod_uint(mbedtls_mpi_uint x, mbedtls_mpi_uint y, mbedtls_mpi_uint b);

unsigned int inline _isnan_(double x);
double _to_double(const mbedtls_mpi *X);
int _from_double(mbedtls_mpi* X, double d);
double _log(mbedtls_mpi *X);

//replaced
size_t mbedtls_mpi_lsb(const mbedtls_mpi *X);
size_t mbedtls_clz(mbedtls_mpi_uint x);
inline int mbedtls_mpi_get_bit(const mbedtls_mpi *X, size_t pos);
int mbedtls_mpi_shift_l(mbedtls_mpi *X, size_t count);
int mbedtls_mpi_shift_r(mbedtls_mpi *X, size_t count);
int mbedtls_mpi_write_string(const mbedtls_mpi *X, int radix, char *buf, size_t buflen, size_t *olen);
//mbedtls_mpi_mod_int() replaced by mbedtls_mpi_mod_uint()
//new added
int mbedtls_udbl_div_uint(const mbedtls_t_udbl *X, const mbedtls_mpi_uint y, mbedtls_t_udbl *Q, mbedtls_mpi_uint *r);
int mbedtls_mpi_div_uint(mbedtls_mpi* X, mbedtls_mpi_uint y, mbedtls_mpi_uint *r);
int mbedtls_mpi_mod_uint(mbedtls_mpi_uint *r, const mbedtls_mpi *A, mbedtls_mpi_uint b);
int mbedtls_mpi_sqrt_i(mbedtls_mpi *X, const mbedtls_mpi *A);
int mbedtls_mpi_pow(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *E);
int mbedtls_mpi_mod2n(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi_sint n);
int mbedtls_mpi_mulmod2n(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *B, const mbedtls_mpi_sint n);
int mbedtls_mpi_invmod2n(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi_sint n, const mbedtls_mpi *C);
int mbedtls_mpi_expmod2n(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *E, const mbedtls_mpi_sint n);
int mbedtls_mpi_expmod_full(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *E, const mbedtls_mpi *N);
//int mbedtls_mpi_inv_mod_DIV(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *N);
int mbedtls_mpi_invmod_hlp(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *B, const mbedtls_mpi *C);
int mbedtls_mpi_invmod_full(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *N, const mbedtls_mpi *C);

#ifdef __cplusplus
}
#endif
