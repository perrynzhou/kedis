/* crc32c.c -- compute CRC-32C using the Intel crc32 instruction
 * Copyright (C) 2013 Mark Adler
 * Version 1.1  1 Aug 2013  Mark Adler
 */

/*
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler
  madler@alumni.caltech.edu
 */

/* Use hardware CRC instruction on Intel SSE 4.2 processors.  This computes a
   CRC-32C, *not* the CRC-32 used by Ethernet and zip, gzip, etc.  A software
   version is provided as a fall-back, as well as for speed comparisons. */

/* Version history:
   1.0  10 Feb 2013  First version
   1.1   1 Aug 2013  Correct comments on why three crc instructions in parallel
   1.2         2020  Added gcc intrinsics, fixed alignment issues.
 */

#include "stl_crc.h"

#include <stddef.h>

/* CRC-32C (iSCSI) polynomial in reversed bit order. */
#define CRC32_POLY 0x82f63b78

#ifdef HAVE_CRC32C
#include <memory.h>
#include <x86intrin.h>

/* Multiply a matrix times a vector over the Galois field of two elements,
   GF(2).  Each element is a bit in an unsigned integer.  mat must have at
   least as many entries as the power of two for most significant one bit in
   vec. */
static inline uint32_t gf2_matrix_times(uint32_t *mat, uint32_t vec)
{
    uint32_t sum = 0;

    while (vec) {
        if (vec & 1) {
            sum ^= *mat;
        }

        vec >>= 1;
        mat++;
    }

    return sum;
}

/* Multiply a matrix by itself over GF(2).  Both mat and square must have 32
   rows. */
static inline void gf2_matrix_square(uint32_t *square, uint32_t *mat)
{
    for (int n = 0; n < 32; n++) {
        square[n] = gf2_matrix_times(mat, mat[n]);
    }
}

/* Construct an operator to apply len zeros to a crc.  len must be a power of
   two.  If len is not a power of two, then the result is the same as for the
   largest power of two less than len.  The result for len == 0 is the same as
   for len == 1.  A version of this routine could be easily written for any
   len, but that is not needed for this application. */
static void crc32_zeros_op(uint32_t *even, size_t len)
{
    uint32_t row = 1;
    uint32_t odd[32]; /* odd-power-of-two zeros operator */

    /* put operator for one zero bit in odd */
    odd[0] = CRC32_POLY; /* CRC-32C polynomial */

    for (int n = 1; n < 32; n++) {
        odd[n] = row;
        row <<= 1;
    }

    /* put operator for two zero bits in even */
    gf2_matrix_square(even, odd);

    /* put operator for four zero bits in odd */
    gf2_matrix_square(odd, even);

    /* first square will put the operator for one zero byte (eight zero bits),
       in even -- next square puts operator for two zero bytes in odd, and so
       on, until len has been rotated down to zero */
    do {
        gf2_matrix_square(even, odd);
        len >>= 1;
        if (len == 0) {
            return;
        }
        gf2_matrix_square(odd, even);
        len >>= 1;
    } while (len);

    /* answer ended up in odd -- copy to even */
    for (int n = 0; n < 32; n++) {
        even[n] = odd[n];
    }
}

/* Take a length and build four lookup tables for applying the zeros operator
   for that length, byte-by-byte on the operand. */
static void crc32_zeros(uint32_t zeros[][256], size_t len)
{
    uint32_t op[32];

    crc32_zeros_op(op, len);

    for (uint32_t n = 0; n < 256; n++) {
        zeros[0][n] = gf2_matrix_times(op, n);
        zeros[1][n] = gf2_matrix_times(op, n << 8);
        zeros[2][n] = gf2_matrix_times(op, n << 16);
        zeros[3][n] = gf2_matrix_times(op, n << 24);
    }
}

/* Apply the zeros operator table to crc. */
static inline uint32_t crc32_shift(uint32_t zeros[][256], uint32_t crc)
{
    return zeros[0][(crc >> 0) & 0xff] ^ zeros[1][(crc >> 8) & 0xff] ^
           zeros[2][(crc >> 16) & 0xff] ^ zeros[3][(crc >> 24) & 0xff];
}

/* Block sizes for three-way parallel crc computation.  LONG and SHORT must
   both be powers of two.  The associated string constants must be set
   accordingly, for use in constructing the assembler instructions. */
#define CRC32_LONG  2048
#define CRC32_SHORT 256

static uint32_t crc32c_long[4][256];
static uint32_t crc32c_short[4][256];

static void crc32_init_hw(void)
{
    crc32_zeros(crc32c_long, CRC32_LONG);
    crc32_zeros(crc32c_short, CRC32_SHORT);
}

uint32_t crc32_hw(uint32_t crc, const uint8_t *buf, uint32_t len)
{
    const unsigned char *next = buf;
    const unsigned char *end;
    uint64_t crc0, crc1, crc2; /* need to be 64 bits for crc32q */

    /* pre-process the crc */
    crc0 = crc ^ 0xffffffff;

    /* compute the crc for up to seven leading bytes to bring the data pointer
       to an eight-byte boundary */
    while (len && ((uintptr_t) next & 7) != 0) {
        crc0 = _mm_crc32_u8(crc0, *next);
        next++;
        len--;
    }

    /* compute the crc on sets of LONG*3 bytes, executing three independent crc
       instructions, each on LONG bytes -- this is optimized for the Nehalem,
       Westmere, Sandy Bridge, and Ivy Bridge architectures, which have a
       throughput of one crc per cycle, but a latency of three cycles */
    while (len >= CRC32_LONG * 3) {
        crc1 = 0;
        crc2 = 0;

        end = next + CRC32_LONG;
        do {
            uint64_t a, b, c;

            memcpy(&a, next, 8);
            memcpy(&b, next + CRC32_LONG, 8);
            memcpy(&c, next + (CRC32_LONG * 2), 8);

            crc0 = _mm_crc32_u64(crc0, a);
            crc1 = _mm_crc32_u64(crc1, b);
            crc2 = _mm_crc32_u64(crc2, c);

            next += 8;
        } while (next < end);

        crc0 = crc32_shift(crc32c_long, crc0) ^ crc1;
        crc0 = crc32_shift(crc32c_long, crc0) ^ crc2;

        next += (CRC32_LONG * 2);
        len -= (CRC32_LONG * 3);
    }

    /* do the same thing, but now on SHORT * 3 blocks for the remaining data
       less than a LONG * 3 block */
    while (len >= CRC32_SHORT * 3) {
        crc1 = 0;
        crc2 = 0;

        end = next + CRC32_SHORT;
        do {
            uint64_t a, b, c;

            memcpy(&a, next, 8);
            memcpy(&b, next + CRC32_SHORT, 8);
            memcpy(&c, next + (CRC32_SHORT * 2), 8);

            crc0 = _mm_crc32_u64(crc0, a);
            crc1 = _mm_crc32_u64(crc1, b);
            crc2 = _mm_crc32_u64(crc2, c);

            next += 8;
        } while (next < end);

        crc0 = crc32_shift(crc32c_short, crc0) ^ crc1;
        crc0 = crc32_shift(crc32c_short, crc0) ^ crc2;

        next += (CRC32_SHORT * 2);
        len -= (CRC32_SHORT * 3);
    }

    /* compute the crc on the remaining eight-byte units less than a SHORT * 3
       block */
    end = next + (len - (len & 7));
    while (next < end) {
        uint64_t a;

        memcpy(&a, next, 8);
        crc0 = _mm_crc32_u64(crc0, a);
        next += 8;
    }
    len &= 7;

    /* compute the crc for up to seven trailing bytes */
    while (len) {
        crc0 = _mm_crc32_u8(crc0, *next);
        next++;
        len--;
    }

    /* return a post-processed crc */
    return (uint32_t) crc0 ^ 0xffffffff;
}

#else

static const uint64_t crc64_tab[256] = {
    0x0000000000000000ULL, 0x7ad870c830358979ULL,
    0xf5b0e190606b12f2ULL, 0x8f689158505e9b8bULL,
    0xc038e5739841b68fULL, 0xbae095bba8743ff6ULL,
    0x358804e3f82aa47dULL, 0x4f50742bc81f2d04ULL,
    0xab28ecb46814fe75ULL, 0xd1f09c7c5821770cULL,
    0x5e980d24087fec87ULL, 0x24407dec384a65feULL,
    0x6b1009c7f05548faULL, 0x11c8790fc060c183ULL,
    0x9ea0e857903e5a08ULL, 0xe478989fa00bd371ULL,
    0x7d08ff3b88be6f81ULL, 0x07d08ff3b88be6f8ULL,
    0x88b81eabe8d57d73ULL, 0xf2606e63d8e0f40aULL,
    0xbd301a4810ffd90eULL, 0xc7e86a8020ca5077ULL,
    0x4880fbd87094cbfcULL, 0x32588b1040a14285ULL,
    0xd620138fe0aa91f4ULL, 0xacf86347d09f188dULL,
    0x2390f21f80c18306ULL, 0x594882d7b0f40a7fULL,
    0x1618f6fc78eb277bULL, 0x6cc0863448deae02ULL,
    0xe3a8176c18803589ULL, 0x997067a428b5bcf0ULL,
    0xfa11fe77117cdf02ULL, 0x80c98ebf2149567bULL,
    0x0fa11fe77117cdf0ULL, 0x75796f2f41224489ULL,
    0x3a291b04893d698dULL, 0x40f16bccb908e0f4ULL,
    0xcf99fa94e9567b7fULL, 0xb5418a5cd963f206ULL,
    0x513912c379682177ULL, 0x2be1620b495da80eULL,
    0xa489f35319033385ULL, 0xde51839b2936bafcULL,
    0x9101f7b0e12997f8ULL, 0xebd98778d11c1e81ULL,
    0x64b116208142850aULL, 0x1e6966e8b1770c73ULL,
    0x8719014c99c2b083ULL, 0xfdc17184a9f739faULL,
    0x72a9e0dcf9a9a271ULL, 0x08719014c99c2b08ULL,
    0x4721e43f0183060cULL, 0x3df994f731b68f75ULL,
    0xb29105af61e814feULL, 0xc849756751dd9d87ULL,
    0x2c31edf8f1d64ef6ULL, 0x56e99d30c1e3c78fULL,
    0xd9810c6891bd5c04ULL, 0xa3597ca0a188d57dULL,
    0xec09088b6997f879ULL, 0x96d1784359a27100ULL,
    0x19b9e91b09fcea8bULL, 0x636199d339c963f2ULL,
    0xdf7adabd7a6e2d6fULL, 0xa5a2aa754a5ba416ULL,
    0x2aca3b2d1a053f9dULL, 0x50124be52a30b6e4ULL,
    0x1f423fcee22f9be0ULL, 0x659a4f06d21a1299ULL,
    0xeaf2de5e82448912ULL, 0x902aae96b271006bULL,
    0x74523609127ad31aULL, 0x0e8a46c1224f5a63ULL,
    0x81e2d7997211c1e8ULL, 0xfb3aa75142244891ULL,
    0xb46ad37a8a3b6595ULL, 0xceb2a3b2ba0eececULL,
    0x41da32eaea507767ULL, 0x3b024222da65fe1eULL,
    0xa2722586f2d042eeULL, 0xd8aa554ec2e5cb97ULL,
    0x57c2c41692bb501cULL, 0x2d1ab4dea28ed965ULL,
    0x624ac0f56a91f461ULL, 0x1892b03d5aa47d18ULL,
    0x97fa21650afae693ULL, 0xed2251ad3acf6feaULL,
    0x095ac9329ac4bc9bULL, 0x7382b9faaaf135e2ULL,
    0xfcea28a2faafae69ULL, 0x8632586aca9a2710ULL,
    0xc9622c4102850a14ULL, 0xb3ba5c8932b0836dULL,
    0x3cd2cdd162ee18e6ULL, 0x460abd1952db919fULL,
    0x256b24ca6b12f26dULL, 0x5fb354025b277b14ULL,
    0xd0dbc55a0b79e09fULL, 0xaa03b5923b4c69e6ULL,
    0xe553c1b9f35344e2ULL, 0x9f8bb171c366cd9bULL,
    0x10e3202993385610ULL, 0x6a3b50e1a30ddf69ULL,
    0x8e43c87e03060c18ULL, 0xf49bb8b633338561ULL,
    0x7bf329ee636d1eeaULL, 0x012b592653589793ULL,
    0x4e7b2d0d9b47ba97ULL, 0x34a35dc5ab7233eeULL,
    0xbbcbcc9dfb2ca865ULL, 0xc113bc55cb19211cULL,
    0x5863dbf1e3ac9decULL, 0x22bbab39d3991495ULL,
    0xadd33a6183c78f1eULL, 0xd70b4aa9b3f20667ULL,
    0x985b3e827bed2b63ULL, 0xe2834e4a4bd8a21aULL,
    0x6debdf121b863991ULL, 0x1733afda2bb3b0e8ULL,
    0xf34b37458bb86399ULL, 0x8993478dbb8deae0ULL,
    0x06fbd6d5ebd3716bULL, 0x7c23a61ddbe6f812ULL,
    0x3373d23613f9d516ULL, 0x49aba2fe23cc5c6fULL,
    0xc6c333a67392c7e4ULL, 0xbc1b436e43a74e9dULL,
    0x95ac9329ac4bc9b5ULL, 0xef74e3e19c7e40ccULL,
    0x601c72b9cc20db47ULL, 0x1ac40271fc15523eULL,
    0x5594765a340a7f3aULL, 0x2f4c0692043ff643ULL,
    0xa02497ca54616dc8ULL, 0xdafce7026454e4b1ULL,
    0x3e847f9dc45f37c0ULL, 0x445c0f55f46abeb9ULL,
    0xcb349e0da4342532ULL, 0xb1eceec59401ac4bULL,
    0xfebc9aee5c1e814fULL, 0x8464ea266c2b0836ULL,
    0x0b0c7b7e3c7593bdULL, 0x71d40bb60c401ac4ULL,
    0xe8a46c1224f5a634ULL, 0x927c1cda14c02f4dULL,
    0x1d148d82449eb4c6ULL, 0x67ccfd4a74ab3dbfULL,
    0x289c8961bcb410bbULL, 0x5244f9a98c8199c2ULL,
    0xdd2c68f1dcdf0249ULL, 0xa7f41839ecea8b30ULL,
    0x438c80a64ce15841ULL, 0x3954f06e7cd4d138ULL,
    0xb63c61362c8a4ab3ULL, 0xcce411fe1cbfc3caULL,
    0x83b465d5d4a0eeceULL, 0xf96c151de49567b7ULL,
    0x76048445b4cbfc3cULL, 0x0cdcf48d84fe7545ULL,
    0x6fbd6d5ebd3716b7ULL, 0x15651d968d029fceULL,
    0x9a0d8ccedd5c0445ULL, 0xe0d5fc06ed698d3cULL,
    0xaf85882d2576a038ULL, 0xd55df8e515432941ULL,
    0x5a3569bd451db2caULL, 0x20ed197575283bb3ULL,
    0xc49581ead523e8c2ULL, 0xbe4df122e51661bbULL,
    0x3125607ab548fa30ULL, 0x4bfd10b2857d7349ULL,
    0x04ad64994d625e4dULL, 0x7e7514517d57d734ULL,
    0xf11d85092d094cbfULL, 0x8bc5f5c11d3cc5c6ULL,
    0x12b5926535897936ULL, 0x686de2ad05bcf04fULL,
    0xe70573f555e26bc4ULL, 0x9ddd033d65d7e2bdULL,
    0xd28d7716adc8cfb9ULL, 0xa85507de9dfd46c0ULL,
    0x273d9686cda3dd4bULL, 0x5de5e64efd965432ULL,
    0xb99d7ed15d9d8743ULL, 0xc3450e196da80e3aULL,
    0x4c2d9f413df695b1ULL, 0x36f5ef890dc31cc8ULL,
    0x79a59ba2c5dc31ccULL, 0x037deb6af5e9b8b5ULL,
    0x8c157a32a5b7233eULL, 0xf6cd0afa9582aa47ULL,
    0x4ad64994d625e4daULL, 0x300e395ce6106da3ULL,
    0xbf66a804b64ef628ULL, 0xc5bed8cc867b7f51ULL,
    0x8aeeace74e645255ULL, 0xf036dc2f7e51db2cULL,
    0x7f5e4d772e0f40a7ULL, 0x05863dbf1e3ac9deULL,
    0xe1fea520be311aafULL, 0x9b26d5e88e0493d6ULL,
    0x144e44b0de5a085dULL, 0x6e963478ee6f8124ULL,
    0x21c640532670ac20ULL, 0x5b1e309b16452559ULL,
    0xd476a1c3461bbed2ULL, 0xaeaed10b762e37abULL,
    0x37deb6af5e9b8b5bULL, 0x4d06c6676eae0222ULL,
    0xc26e573f3ef099a9ULL, 0xb8b627f70ec510d0ULL,
    0xf7e653dcc6da3dd4ULL, 0x8d3e2314f6efb4adULL,
    0x0256b24ca6b12f26ULL, 0x788ec2849684a65fULL,
    0x9cf65a1b368f752eULL, 0xe62e2ad306bafc57ULL,
    0x6946bb8b56e467dcULL, 0x139ecb4366d1eea5ULL,
    0x5ccebf68aecec3a1ULL, 0x2616cfa09efb4ad8ULL,
    0xa97e5ef8cea5d153ULL, 0xd3a62e30fe90582aULL,
    0xb0c7b7e3c7593bd8ULL, 0xca1fc72bf76cb2a1ULL,
    0x45775673a732292aULL, 0x3faf26bb9707a053ULL,
    0x70ff52905f188d57ULL, 0x0a2722586f2d042eULL,
    0x854fb3003f739fa5ULL, 0xff97c3c80f4616dcULL,
    0x1bef5b57af4dc5adULL, 0x61372b9f9f784cd4ULL,
    0xee5fbac7cf26d75fULL, 0x9487ca0fff135e26ULL,
    0xdbd7be24370c7322ULL, 0xa10fceec0739fa5bULL,
    0x2e675fb4576761d0ULL, 0x54bf2f7c6752e8a9ULL,
    0xcdcf48d84fe75459ULL, 0xb71738107fd2dd20ULL,
    0x387fa9482f8c46abULL, 0x42a7d9801fb9cfd2ULL,
    0x0df7adabd7a6e2d6ULL, 0x772fdd63e7936bafULL,
    0xf8474c3bb7cdf024ULL, 0x829f3cf387f8795dULL,
    0x66e7a46c27f3aa2cULL, 0x1c3fd4a417c62355ULL,
    0x935745fc4798b8deULL, 0xe98f353477ad31a7ULL,
    0xa6df411fbfb21ca3ULL, 0xdc0731d78f8795daULL,
    0x536fa08fdfd90e51ULL, 0x29b7d047efec8728ULL,
};
static const uint16_t crc16tab[256]= {
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
    0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
    0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
    0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
    0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
    0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
    0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
    0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
    0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
    0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
    0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
    0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
    0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
    0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
    0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
    0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
    0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
    0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
    0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
    0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
    0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
    0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
    0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
    0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
    0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
    0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
    0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
    0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
    0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
    0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
    0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
    0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};
/* Table for a quadword-at-a-time software crc. */
static uint32_t crc32c_table[8][256];

/* Construct table for software CRC-32C calculation. */
static void crc32_init_sw(void)
{
    for (uint32_t n = 0; n < 256; n++) {
        uint32_t crc = n;

        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;

        crc32c_table[0][n] = crc;
    }

    for (uint32_t n = 0; n < 256; n++) {
        uint32_t crc = crc32c_table[0][n];

        for (uint32_t k = 1; k < 8; k++) {
            crc = crc32c_table[0][crc & 0xff] ^ (crc >> 8);
            crc32c_table[k][n] = crc;
        }
    }
}
uint64_t stl_crc64(const char *s, int len) {
    int j = 0;
    uint64_t crc = 0;

    for (j = 0; j < len; j++) {
        uint8_t byte = s[j];
        crc = crc64_tab[(uint8_t)crc ^ byte] ^ (crc >> 8);
    }
    return crc;
}


uint16_t stl_crc16(const char *buf, int len) {
    int counter;
    uint16_t crc = 0;
    for (counter = 0; counter < len; counter++){
        crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *buf++)&0x00FF];
    }
    return crc;
}

/* Table-driven software version as a fall-back.  This is about 15 times slower
   than using the hardware instructions.  This assumes little-endian integers,
   as is the case on Intel processors that the assembler code here is for. */
static uint32_t crc32_sw(uint32_t crci, const void *buf, size_t len)
{
    const unsigned char *next = buf;
    uint64_t crc = crci ^ 0xffffffff;

    while (len && ((uintptr_t) next & 7) != 0) {
        crc = crc32c_table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        len--;
    }

    while (len >= 8) {
        crc ^= *(uint64_t *) next;
        crc = crc32c_table[7][crc & 0xff] ^ crc32c_table[6][(crc >> 8) & 0xff] ^
              crc32c_table[5][(crc >> 16) & 0xff] ^
              crc32c_table[4][(crc >> 24) & 0xff] ^
              crc32c_table[3][(crc >> 32) & 0xff] ^
              crc32c_table[2][(crc >> 40) & 0xff] ^
              crc32c_table[1][(crc >> 48) & 0xff] ^ crc32c_table[0][crc >> 56];
        next += 8;
        len -= 8;
    }

    while (len) {
        crc = crc32c_table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        len--;
    }

    return (uint32_t) crc ^ 0xffffffff;
}

#endif

uint32_t stl_crc32(uint32_t crc, const uint8_t *buf, uint32_t len)
{
#ifdef HAVE_CRC32C
    return crc32_hw(crc, buf, len);
#else
    return crc32_sw(crc, buf, len);
#endif
}

void stl_crc32_init()
{
#ifdef HAVE_CRC32C
    crc32_init_hw();
#else
    crc32_init_sw();
#endif
}
