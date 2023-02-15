/*
* Camellia
* (C) 2012,2020 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/internal/camellia.h>
#include <botan/internal/loadstor.h>
#include <botan/internal/rotate.h>

namespace Botan {

namespace {

namespace Camellia_F {

alignas(256) const uint8_t SBOX1[256] = {
   0x70, 0x82, 0x2C, 0xEC, 0xB3, 0x27, 0xC0, 0xE5, 0xE4, 0x85, 0x57,
   0x35, 0xEA, 0x0C, 0xAE, 0x41, 0x23, 0xEF, 0x6B, 0x93, 0x45, 0x19,
   0xA5, 0x21, 0xED, 0x0E, 0x4F, 0x4E, 0x1D, 0x65, 0x92, 0xBD, 0x86,
   0xB8, 0xAF, 0x8F, 0x7C, 0xEB, 0x1F, 0xCE, 0x3E, 0x30, 0xDC, 0x5F,
   0x5E, 0xC5, 0x0B, 0x1A, 0xA6, 0xE1, 0x39, 0xCA, 0xD5, 0x47, 0x5D,
   0x3D, 0xD9, 0x01, 0x5A, 0xD6, 0x51, 0x56, 0x6C, 0x4D, 0x8B, 0x0D,
   0x9A, 0x66, 0xFB, 0xCC, 0xB0, 0x2D, 0x74, 0x12, 0x2B, 0x20, 0xF0,
   0xB1, 0x84, 0x99, 0xDF, 0x4C, 0xCB, 0xC2, 0x34, 0x7E, 0x76, 0x05,
   0x6D, 0xB7, 0xA9, 0x31, 0xD1, 0x17, 0x04, 0xD7, 0x14, 0x58, 0x3A,
   0x61, 0xDE, 0x1B, 0x11, 0x1C, 0x32, 0x0F, 0x9C, 0x16, 0x53, 0x18,
   0xF2, 0x22, 0xFE, 0x44, 0xCF, 0xB2, 0xC3, 0xB5, 0x7A, 0x91, 0x24,
   0x08, 0xE8, 0xA8, 0x60, 0xFC, 0x69, 0x50, 0xAA, 0xD0, 0xA0, 0x7D,
   0xA1, 0x89, 0x62, 0x97, 0x54, 0x5B, 0x1E, 0x95, 0xE0, 0xFF, 0x64,
   0xD2, 0x10, 0xC4, 0x00, 0x48, 0xA3, 0xF7, 0x75, 0xDB, 0x8A, 0x03,
   0xE6, 0xDA, 0x09, 0x3F, 0xDD, 0x94, 0x87, 0x5C, 0x83, 0x02, 0xCD,
   0x4A, 0x90, 0x33, 0x73, 0x67, 0xF6, 0xF3, 0x9D, 0x7F, 0xBF, 0xE2,
   0x52, 0x9B, 0xD8, 0x26, 0xC8, 0x37, 0xC6, 0x3B, 0x81, 0x96, 0x6F,
   0x4B, 0x13, 0xBE, 0x63, 0x2E, 0xE9, 0x79, 0xA7, 0x8C, 0x9F, 0x6E,
   0xBC, 0x8E, 0x29, 0xF5, 0xF9, 0xB6, 0x2F, 0xFD, 0xB4, 0x59, 0x78,
   0x98, 0x06, 0x6A, 0xE7, 0x46, 0x71, 0xBA, 0xD4, 0x25, 0xAB, 0x42,
   0x88, 0xA2, 0x8D, 0xFA, 0x72, 0x07, 0xB9, 0x55, 0xF8, 0xEE, 0xAC,
   0x0A, 0x36, 0x49, 0x2A, 0x68, 0x3C, 0x38, 0xF1, 0xA4, 0x40, 0x28,
   0xD3, 0x7B, 0xBB, 0xC9, 0x43, 0xC1, 0x15, 0xE3, 0xAD, 0xF4, 0x77,
   0xC7, 0x80, 0x9E };

// SBOX2[x] = rotl<1>(SBOX1[x])
alignas(256) const uint8_t SBOX2[256] = {
   0xE0, 0x05, 0x58, 0xD9, 0x67, 0x4E, 0x81, 0xCB, 0xC9, 0x0B, 0xAE,
   0x6A, 0xD5, 0x18, 0x5D, 0x82, 0x46, 0xDF, 0xD6, 0x27, 0x8A, 0x32,
   0x4B, 0x42, 0xDB, 0x1C, 0x9E, 0x9C, 0x3A, 0xCA, 0x25, 0x7B, 0x0D,
   0x71, 0x5F, 0x1F, 0xF8, 0xD7, 0x3E, 0x9D, 0x7C, 0x60, 0xB9, 0xBE,
   0xBC, 0x8B, 0x16, 0x34, 0x4D, 0xC3, 0x72, 0x95, 0xAB, 0x8E, 0xBA,
   0x7A, 0xB3, 0x02, 0xB4, 0xAD, 0xA2, 0xAC, 0xD8, 0x9A, 0x17, 0x1A,
   0x35, 0xCC, 0xF7, 0x99, 0x61, 0x5A, 0xE8, 0x24, 0x56, 0x40, 0xE1,
   0x63, 0x09, 0x33, 0xBF, 0x98, 0x97, 0x85, 0x68, 0xFC, 0xEC, 0x0A,
   0xDA, 0x6F, 0x53, 0x62, 0xA3, 0x2E, 0x08, 0xAF, 0x28, 0xB0, 0x74,
   0xC2, 0xBD, 0x36, 0x22, 0x38, 0x64, 0x1E, 0x39, 0x2C, 0xA6, 0x30,
   0xE5, 0x44, 0xFD, 0x88, 0x9F, 0x65, 0x87, 0x6B, 0xF4, 0x23, 0x48,
   0x10, 0xD1, 0x51, 0xC0, 0xF9, 0xD2, 0xA0, 0x55, 0xA1, 0x41, 0xFA,
   0x43, 0x13, 0xC4, 0x2F, 0xA8, 0xB6, 0x3C, 0x2B, 0xC1, 0xFF, 0xC8,
   0xA5, 0x20, 0x89, 0x00, 0x90, 0x47, 0xEF, 0xEA, 0xB7, 0x15, 0x06,
   0xCD, 0xB5, 0x12, 0x7E, 0xBB, 0x29, 0x0F, 0xB8, 0x07, 0x04, 0x9B,
   0x94, 0x21, 0x66, 0xE6, 0xCE, 0xED, 0xE7, 0x3B, 0xFE, 0x7F, 0xC5,
   0xA4, 0x37, 0xB1, 0x4C, 0x91, 0x6E, 0x8D, 0x76, 0x03, 0x2D, 0xDE,
   0x96, 0x26, 0x7D, 0xC6, 0x5C, 0xD3, 0xF2, 0x4F, 0x19, 0x3F, 0xDC,
   0x79, 0x1D, 0x52, 0xEB, 0xF3, 0x6D, 0x5E, 0xFB, 0x69, 0xB2, 0xF0,
   0x31, 0x0C, 0xD4, 0xCF, 0x8C, 0xE2, 0x75, 0xA9, 0x4A, 0x57, 0x84,
   0x11, 0x45, 0x1B, 0xF5, 0xE4, 0x0E, 0x73, 0xAA, 0xF1, 0xDD, 0x59,
   0x14, 0x6C, 0x92, 0x54, 0xD0, 0x78, 0x70, 0xE3, 0x49, 0x80, 0x50,
   0xA7, 0xF6, 0x77, 0x93, 0x86, 0x83, 0x2A, 0xC7, 0x5B, 0xE9, 0xEE,
   0x8F, 0x01, 0x3D };

// SBOX3[x] = rotl<7>(SBOX1[x])
alignas(256) const uint8_t SBOX3[256] = {
   0x38, 0x41, 0x16, 0x76, 0xD9, 0x93, 0x60, 0xF2, 0x72, 0xC2, 0xAB,
   0x9A, 0x75, 0x06, 0x57, 0xA0, 0x91, 0xF7, 0xB5, 0xC9, 0xA2, 0x8C,
   0xD2, 0x90, 0xF6, 0x07, 0xA7, 0x27, 0x8E, 0xB2, 0x49, 0xDE, 0x43,
   0x5C, 0xD7, 0xC7, 0x3E, 0xF5, 0x8F, 0x67, 0x1F, 0x18, 0x6E, 0xAF,
   0x2F, 0xE2, 0x85, 0x0D, 0x53, 0xF0, 0x9C, 0x65, 0xEA, 0xA3, 0xAE,
   0x9E, 0xEC, 0x80, 0x2D, 0x6B, 0xA8, 0x2B, 0x36, 0xA6, 0xC5, 0x86,
   0x4D, 0x33, 0xFD, 0x66, 0x58, 0x96, 0x3A, 0x09, 0x95, 0x10, 0x78,
   0xD8, 0x42, 0xCC, 0xEF, 0x26, 0xE5, 0x61, 0x1A, 0x3F, 0x3B, 0x82,
   0xB6, 0xDB, 0xD4, 0x98, 0xE8, 0x8B, 0x02, 0xEB, 0x0A, 0x2C, 0x1D,
   0xB0, 0x6F, 0x8D, 0x88, 0x0E, 0x19, 0x87, 0x4E, 0x0B, 0xA9, 0x0C,
   0x79, 0x11, 0x7F, 0x22, 0xE7, 0x59, 0xE1, 0xDA, 0x3D, 0xC8, 0x12,
   0x04, 0x74, 0x54, 0x30, 0x7E, 0xB4, 0x28, 0x55, 0x68, 0x50, 0xBE,
   0xD0, 0xC4, 0x31, 0xCB, 0x2A, 0xAD, 0x0F, 0xCA, 0x70, 0xFF, 0x32,
   0x69, 0x08, 0x62, 0x00, 0x24, 0xD1, 0xFB, 0xBA, 0xED, 0x45, 0x81,
   0x73, 0x6D, 0x84, 0x9F, 0xEE, 0x4A, 0xC3, 0x2E, 0xC1, 0x01, 0xE6,
   0x25, 0x48, 0x99, 0xB9, 0xB3, 0x7B, 0xF9, 0xCE, 0xBF, 0xDF, 0x71,
   0x29, 0xCD, 0x6C, 0x13, 0x64, 0x9B, 0x63, 0x9D, 0xC0, 0x4B, 0xB7,
   0xA5, 0x89, 0x5F, 0xB1, 0x17, 0xF4, 0xBC, 0xD3, 0x46, 0xCF, 0x37,
   0x5E, 0x47, 0x94, 0xFA, 0xFC, 0x5B, 0x97, 0xFE, 0x5A, 0xAC, 0x3C,
   0x4C, 0x03, 0x35, 0xF3, 0x23, 0xB8, 0x5D, 0x6A, 0x92, 0xD5, 0x21,
   0x44, 0x51, 0xC6, 0x7D, 0x39, 0x83, 0xDC, 0xAA, 0x7C, 0x77, 0x56,
   0x05, 0x1B, 0xA4, 0x15, 0x34, 0x1E, 0x1C, 0xF8, 0x52, 0x20, 0x14,
   0xE9, 0xBD, 0xDD, 0xE4, 0xA1, 0xE0, 0x8A, 0xF1, 0xD6, 0x7A, 0xBB,
   0xE3, 0x40, 0x4F };

// SBOX4[x] = SBOX1[rotl<1>(x)]
alignas(256) const uint8_t SBOX4[256] = {
   0x70, 0x2C, 0xB3, 0xC0, 0xE4, 0x57, 0xEA, 0xAE, 0x23, 0x6B, 0x45,
   0xA5, 0xED, 0x4F, 0x1D, 0x92, 0x86, 0xAF, 0x7C, 0x1F, 0x3E, 0xDC,
   0x5E, 0x0B, 0xA6, 0x39, 0xD5, 0x5D, 0xD9, 0x5A, 0x51, 0x6C, 0x8B,
   0x9A, 0xFB, 0xB0, 0x74, 0x2B, 0xF0, 0x84, 0xDF, 0xCB, 0x34, 0x76,
   0x6D, 0xA9, 0xD1, 0x04, 0x14, 0x3A, 0xDE, 0x11, 0x32, 0x9C, 0x53,
   0xF2, 0xFE, 0xCF, 0xC3, 0x7A, 0x24, 0xE8, 0x60, 0x69, 0xAA, 0xA0,
   0xA1, 0x62, 0x54, 0x1E, 0xE0, 0x64, 0x10, 0x00, 0xA3, 0x75, 0x8A,
   0xE6, 0x09, 0xDD, 0x87, 0x83, 0xCD, 0x90, 0x73, 0xF6, 0x9D, 0xBF,
   0x52, 0xD8, 0xC8, 0xC6, 0x81, 0x6F, 0x13, 0x63, 0xE9, 0xA7, 0x9F,
   0xBC, 0x29, 0xF9, 0x2F, 0xB4, 0x78, 0x06, 0xE7, 0x71, 0xD4, 0xAB,
   0x88, 0x8D, 0x72, 0xB9, 0xF8, 0xAC, 0x36, 0x2A, 0x3C, 0xF1, 0x40,
   0xD3, 0xBB, 0x43, 0x15, 0xAD, 0x77, 0x80, 0x82, 0xEC, 0x27, 0xE5,
   0x85, 0x35, 0x0C, 0x41, 0xEF, 0x93, 0x19, 0x21, 0x0E, 0x4E, 0x65,
   0xBD, 0xB8, 0x8F, 0xEB, 0xCE, 0x30, 0x5F, 0xC5, 0x1A, 0xE1, 0xCA,
   0x47, 0x3D, 0x01, 0xD6, 0x56, 0x4D, 0x0D, 0x66, 0xCC, 0x2D, 0x12,
   0x20, 0xB1, 0x99, 0x4C, 0xC2, 0x7E, 0x05, 0xB7, 0x31, 0x17, 0xD7,
   0x58, 0x61, 0x1B, 0x1C, 0x0F, 0x16, 0x18, 0x22, 0x44, 0xB2, 0xB5,
   0x91, 0x08, 0xA8, 0xFC, 0x50, 0xD0, 0x7D, 0x89, 0x97, 0x5B, 0x95,
   0xFF, 0xD2, 0xC4, 0x48, 0xF7, 0xDB, 0x03, 0xDA, 0x3F, 0x94, 0x5C,
   0x02, 0x4A, 0x33, 0x67, 0xF3, 0x7F, 0xE2, 0x9B, 0x26, 0x37, 0x3B,
   0x96, 0x4B, 0xBE, 0x2E, 0x79, 0x8C, 0x6E, 0x8E, 0xF5, 0xB6, 0xFD,
   0x59, 0x98, 0x6A, 0x46, 0xBA, 0x25, 0x42, 0xA2, 0xFA, 0x07, 0x55,
   0xEE, 0x0A, 0x49, 0x68, 0x38, 0xA4, 0x28, 0x7B, 0xC9, 0xC1, 0xE3,
   0xF4, 0xC7, 0x9E };

uint64_t F(uint64_t v, uint64_t K)
   {
   const uint64_t M1 = 0x0101010001000001;
   const uint64_t M2 = 0x0001010101010000;
   const uint64_t M3 = 0x0100010100010100;
   const uint64_t M4 = 0x0101000100000101;
   const uint64_t M5 = 0x0001010100010101;
   const uint64_t M6 = 0x0100010101000101;
   const uint64_t M7 = 0x0101000101010001;
   const uint64_t M8 = 0x0101010001010100;

   const uint64_t x = v ^ K;

   const uint64_t Z1 = M1 * SBOX1[get_byte<0>(x)];
   const uint64_t Z2 = M2 * SBOX2[get_byte<1>(x)];
   const uint64_t Z3 = M3 * SBOX3[get_byte<2>(x)];
   const uint64_t Z4 = M4 * SBOX4[get_byte<3>(x)];
   const uint64_t Z5 = M5 * SBOX2[get_byte<4>(x)];
   const uint64_t Z6 = M6 * SBOX3[get_byte<5>(x)];
   const uint64_t Z7 = M7 * SBOX4[get_byte<6>(x)];
   const uint64_t Z8 = M8 * SBOX1[get_byte<7>(x)];

   return Z1 ^ Z2 ^ Z3 ^ Z4 ^ Z5 ^ Z6 ^ Z7 ^ Z8;
   }

inline uint64_t FL(uint64_t v, uint64_t K)
   {
   uint32_t x1 = static_cast<uint32_t>(v >> 32);
   uint32_t x2 = static_cast<uint32_t>(v & 0xFFFFFFFF);

   const uint32_t k1 = static_cast<uint32_t>(K >> 32);
   const uint32_t k2 = static_cast<uint32_t>(K & 0xFFFFFFFF);

   x2 ^= rotl<1>(x1 & k1);
   x1 ^= (x2 | k2);

   return ((static_cast<uint64_t>(x1) << 32) | x2);
   }

inline uint64_t FLINV(uint64_t v, uint64_t K)
   {
   uint32_t x1 = static_cast<uint32_t>(v >> 32);
   uint32_t x2 = static_cast<uint32_t>(v & 0xFFFFFFFF);

   const uint32_t k1 = static_cast<uint32_t>(K >> 32);
   const uint32_t k2 = static_cast<uint32_t>(K & 0xFFFFFFFF);

   x1 ^= (x2 | k2);
   x2 ^= rotl<1>(x1 & k1);

   return ((static_cast<uint64_t>(x1) << 32) | x2);
   }

/*
* Camellia Encryption
*/
void encrypt(const uint8_t in[], uint8_t out[], size_t blocks,
             const secure_vector<uint64_t>& SK, const size_t rounds)
   {
   for(size_t i = 0; i < blocks; ++i)
      {
      uint64_t D1, D2;
      load_be(in + 16*i, D1, D2);

      const uint64_t* K = SK.data();

      D1 ^= *K++;
      D2 ^= *K++;

      D2 ^= F(D1, *K++);
      D1 ^= F(D2, *K++);

      for(size_t r = 1; r != rounds - 1; ++r)
         {
         if(r % 3 == 0)
            {
            D1 = FL   (D1, *K++);
            D2 = FLINV(D2, *K++);
            }

         D2 ^= F(D1, *K++);
         D1 ^= F(D2, *K++);
         }

      D2 ^= F(D1, *K++);
      D1 ^= F(D2, *K++);

      D2 ^= *K++;
      D1 ^= *K++;

      store_be(out + 16*i, D2, D1);
      }
   }

/*
* Camellia Decryption
*/
void decrypt(const uint8_t in[], uint8_t out[], size_t blocks,
             const secure_vector<uint64_t>& SK, const size_t rounds)
   {
   for(size_t i = 0; i < blocks; ++i)
      {
      uint64_t D1, D2;
      load_be(in + 16*i, D1, D2);

      const uint64_t* K = &SK[SK.size()-1];

      D2 ^= *K--;
      D1 ^= *K--;

      D2 ^= F(D1, *K--);
      D1 ^= F(D2, *K--);

      for(size_t r = 1; r != rounds - 1; ++r)
         {
         if(r % 3 == 0)
            {
            D1 = FL   (D1, *K--);
            D2 = FLINV(D2, *K--);
            }

         D2 ^= F(D1, *K--);
         D1 ^= F(D2, *K--);
         }

      D2 ^= F(D1, *K--);
      D1 ^= F(D2, *K--);

      D1 ^= *K--;
      D2 ^= *K;

      store_be(out + 16*i, D2, D1);
      }
   }

inline uint64_t left_rot_hi(uint64_t h, uint64_t l, size_t shift)
   {
   if(shift >= 64)
      shift -= 64;
   return (h << shift) | (l >> (64-shift));
   }

inline uint64_t left_rot_lo(uint64_t h, uint64_t l, size_t shift)
   {
   if(shift >= 64)
      shift -= 64;
   return (h >> (64-shift)) | (l << shift);
   }

/*
* Camellia Key Schedule
*/
void key_schedule(secure_vector<uint64_t>& SK, const uint8_t key[], size_t length)
   {
   const uint64_t Sigma1 = 0xA09E667F3BCC908B;
   const uint64_t Sigma2 = 0xB67AE8584CAA73B2;
   const uint64_t Sigma3 = 0xC6EF372FE94F82BE;
   const uint64_t Sigma4 = 0x54FF53A5F1D36F1C;
   const uint64_t Sigma5 = 0x10E527FADE682D1D;
   const uint64_t Sigma6 = 0xB05688C2B3E6C1FD;

   const uint64_t KL_H = load_be<uint64_t>(key, 0);
   const uint64_t KL_L = load_be<uint64_t>(key, 1);

   const uint64_t KR_H = (length >= 24) ? load_be<uint64_t>(key, 2) : 0;
   const uint64_t KR_L =
      (length == 32) ? load_be<uint64_t>(key, 3) : ((length == 24) ? ~KR_H : 0);

   uint64_t D1 = KL_H ^ KR_H;
   uint64_t D2 = KL_L ^ KR_L;
   D2 ^= F(D1, Sigma1);
   D1 ^= F(D2, Sigma2);
   D1 ^= KL_H;
   D2 ^= KL_L;
   D2 ^= F(D1, Sigma3);
   D1 ^= F(D2, Sigma4);

   const uint64_t KA_H = D1;
   const uint64_t KA_L = D2;

   D1 = KA_H ^ KR_H;
   D2 = KA_L ^ KR_L;
   D2 ^= F(D1, Sigma5);
   D1 ^= F(D2, Sigma6);

   const uint64_t KB_H = D1;
   const uint64_t KB_L = D2;

   if(length == 16)
      {
      SK.resize(26);

      SK[ 0] = KL_H;
      SK[ 1] = KL_L;
      SK[ 2] = KA_H;
      SK[ 3] = KA_L;
      SK[ 4] = left_rot_hi(KL_H, KL_L, 15);
      SK[ 5] = left_rot_lo(KL_H, KL_L, 15);
      SK[ 6] = left_rot_hi(KA_H, KA_L, 15);
      SK[ 7] = left_rot_lo(KA_H, KA_L, 15);
      SK[ 8] = left_rot_hi(KA_H, KA_L, 30);
      SK[ 9] = left_rot_lo(KA_H, KA_L, 30);
      SK[10] = left_rot_hi(KL_H, KL_L, 45);
      SK[11] = left_rot_lo(KL_H, KL_L, 45);
      SK[12] = left_rot_hi(KA_H, KA_L,  45);
      SK[13] = left_rot_lo(KL_H, KL_L,  60);
      SK[14] = left_rot_hi(KA_H, KA_L,  60);
      SK[15] = left_rot_lo(KA_H, KA_L,  60);
      SK[16] = left_rot_lo(KL_H, KL_L,  77);
      SK[17] = left_rot_hi(KL_H, KL_L,  77);
      SK[18] = left_rot_lo(KL_H, KL_L,  94);
      SK[19] = left_rot_hi(KL_H, KL_L,  94);
      SK[20] = left_rot_lo(KA_H, KA_L,  94);
      SK[21] = left_rot_hi(KA_H, KA_L,  94);
      SK[22] = left_rot_lo(KL_H, KL_L, 111);
      SK[23] = left_rot_hi(KL_H, KL_L, 111);
      SK[24] = left_rot_lo(KA_H, KA_L, 111);
      SK[25] = left_rot_hi(KA_H, KA_L, 111);
      }
   else
      {
      SK.resize(34);

      SK[ 0] = KL_H;
      SK[ 1] = KL_L;
      SK[ 2] = KB_H;
      SK[ 3] = KB_L;

      SK[ 4] = left_rot_hi(KR_H, KR_L, 15);
      SK[ 5] = left_rot_lo(KR_H, KR_L, 15);
      SK[ 6] = left_rot_hi(KA_H, KA_L, 15);
      SK[ 7] = left_rot_lo(KA_H, KA_L, 15);

      SK[ 8] = left_rot_hi(KR_H, KR_L, 30);
      SK[ 9] = left_rot_lo(KR_H, KR_L, 30);
      SK[10] = left_rot_hi(KB_H, KB_L, 30);
      SK[11] = left_rot_lo(KB_H, KB_L, 30);

      SK[12] = left_rot_hi(KL_H, KL_L, 45);
      SK[13] = left_rot_lo(KL_H, KL_L, 45);
      SK[14] = left_rot_hi(KA_H, KA_L, 45);
      SK[15] = left_rot_lo(KA_H, KA_L, 45);

      SK[16] = left_rot_hi(KL_H, KL_L, 60);
      SK[17] = left_rot_lo(KL_H, KL_L, 60);
      SK[18] = left_rot_hi(KR_H, KR_L, 60);
      SK[19] = left_rot_lo(KR_H, KR_L, 60);
      SK[20] = left_rot_hi(KB_H, KB_L, 60);
      SK[21] = left_rot_lo(KB_H, KB_L, 60);

      SK[22] = left_rot_lo(KL_H, KL_L,  77);
      SK[23] = left_rot_hi(KL_H, KL_L,  77);
      SK[24] = left_rot_lo(KA_H, KA_L,  77);
      SK[25] = left_rot_hi(KA_H, KA_L,  77);

      SK[26] = left_rot_lo(KR_H, KR_L,  94);
      SK[27] = left_rot_hi(KR_H, KR_L,  94);
      SK[28] = left_rot_lo(KA_H, KA_L,  94);
      SK[29] = left_rot_hi(KA_H, KA_L,  94);
      SK[30] = left_rot_lo(KL_H, KL_L, 111);
      SK[31] = left_rot_hi(KL_H, KL_L, 111);
      SK[32] = left_rot_lo(KB_H, KB_L, 111);
      SK[33] = left_rot_hi(KB_H, KB_L, 111);
      }
   }

}

}

void Camellia_128::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   verify_key_set(m_SK.empty() == false);
   Camellia_F::encrypt(in, out, blocks, m_SK, 9);
   }

void Camellia_192::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   verify_key_set(m_SK.empty() == false);
   Camellia_F::encrypt(in, out, blocks, m_SK, 12);
   }

void Camellia_256::encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   verify_key_set(m_SK.empty() == false);
   Camellia_F::encrypt(in, out, blocks, m_SK, 12);
   }

void Camellia_128::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   verify_key_set(m_SK.empty() == false);
   Camellia_F::decrypt(in, out, blocks, m_SK, 9);
   }

void Camellia_192::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   verify_key_set(m_SK.empty() == false);
   Camellia_F::decrypt(in, out, blocks, m_SK, 12);
   }

void Camellia_256::decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   verify_key_set(m_SK.empty() == false);
   Camellia_F::decrypt(in, out, blocks, m_SK, 12);
   }

void Camellia_128::key_schedule(const uint8_t key[], size_t length)
   {
   Camellia_F::key_schedule(m_SK, key, length);
   }

void Camellia_192::key_schedule(const uint8_t key[], size_t length)
   {
   Camellia_F::key_schedule(m_SK, key, length);
   }

void Camellia_256::key_schedule(const uint8_t key[], size_t length)
   {
   Camellia_F::key_schedule(m_SK, key, length);
   }

void Camellia_128::clear()
   {
   zap(m_SK);
   }

void Camellia_192::clear()
   {
   zap(m_SK);
   }

void Camellia_256::clear()
   {
   zap(m_SK);
   }

}
