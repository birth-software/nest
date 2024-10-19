#include <std/sha1.h>
// https://github.com/jasinb/sha1.git
// STRUCT(Sha1Digest)
// {
//     u32 digest[5];
// };

// static uint32_t rotl32(uint32_t x, int b)
// {
//     return (x << b) | (x >> (32-b));
// }
//
// switch endianness
// fn u32 sha1_get32(u8* p)
// {
//     return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
// }

// fn u32 sha1_f(int t, u32 b, u32 c, u32 d)
// {
//     assert(0 <= t && t < 80);
//
//     if (t < 20)
//     {
//         return (b & c) | ((~b) & d);
//     }
//     else if (t < 40)
//     {
//         return b ^ c ^ d;
//     }
//     else if (t < 60)
//     {
//         return (b & c) | (b & d) | (c & d);
//     }
//     else
//     //if (t < 80)
//     {
//         return b ^ c ^ d;
//     }
// }

// STRUCT(Sha1Context)
// {
//     u8 block[64];
//     u32 h[5];
//     u64 bytes;
//     u32 cur;
// };

// fn void sha1_reset(Sha1Context* ctx)
// {
//     ctx->h[0] = 0x67452301;
//     ctx->h[1] = 0xefcdab89;
//     ctx->h[2] = 0x98badcfe;
//     ctx->h[3] = 0x10325476;
//     ctx->h[4] = 0xc3d2e1f0;
//     ctx->bytes = 0;
//     ctx->cur = 0;
// }

// fn void sha1_process_block(Sha1Context* ctx)
// {
//     global const u32 k[4] =
//     {
//         0x5A827999,
//         0x6ED9EBA1,
//         0x8F1BBCDC,
//         0xCA62C1D6
//     };
//
//     u32 w[16];
//     u32 a = ctx->h[0];
//     u32 b = ctx->h[1];
//     u32 c = ctx->h[2];
//     u32 d = ctx->h[3];
//     u32 e = ctx->h[4];
//     u32 t;
//
//     for (t = 0; t < 16; t++)
//         w[t] = sha1_get32((u8*)(&((uint32_t*)ctx->block)[t]));
//
//     for (t = 0; t < 80; t++)
//     {
//         auto s = t & 0xf;
//         u32 temp;
//         if (t >= 16)
//             w[s] = rotate_left_u32(w[(s + 13) & 0xf] ^ w[(s + 8) & 0xf] ^ w[(s + 2) & 0xf] ^ w[s], 1);
//
//          temp = rotate_left_u32(a, 5) + sha1_f(t, b,c,d) + e + w[s] + k[t/20];
//
//          e = d; d = c; c = rotate_left_u32(b, 30); b = a; a = temp;
//     }
//
//     ctx->h[0] += a;
//     ctx->h[1] += b;
//     ctx->h[2] += c;
//     ctx->h[3] += d;
//     ctx->h[4] += e;
// }

// fn void sha1_write(Sha1Context* ctx, String bytes)
// {
//     auto length = bytes.length;
//     ctx->bytes += length;
//
//     const uint8_t* src = bytes.pointer;
//     while (length--)
//     {
//         // TODO: could optimize the first and last few bytes, and then copy
//         // 128 bit blocks with SIMD in between
//         ctx->block[ctx->cur++] = *src++;
//         if (ctx->cur == 64)
//         {
//             sha1_process_block(ctx);
//             ctx->cur = 0;
//         }
//     }
// }

// fn Sha1Digest sha1_get_digest(Sha1Context* ctx)
// {
//     // append separator
//     ctx->block[ctx->cur++] = 0x80;
//     if (ctx->cur > 56)
//     {
//         // no space in block for the 64-bit message length, flush
//         memset(&ctx->block[ctx->cur], 0, 64 - ctx->cur);
//         sha1_process_block(ctx);
//         ctx->cur = 0;
//     }
//
//     memset(&ctx->block[ctx->cur], 0, 56 - ctx->cur);
//     uint64_t bits = ctx->bytes * 8;
//
//     // TODO a few instructions could be shaven
//     ctx->block[56] = (uint8_t)(bits >> 56 & 0xff);
//     ctx->block[57] = (uint8_t)(bits >> 48 & 0xff);
//     ctx->block[58] = (uint8_t)(bits >> 40 & 0xff);
//     ctx->block[59] = (uint8_t)(bits >> 32 & 0xff);
//     ctx->block[60] = (uint8_t)(bits >> 24 & 0xff);
//     ctx->block[61] = (uint8_t)(bits >> 16 & 0xff);
//     ctx->block[62] = (uint8_t)(bits >> 8  & 0xff);
//     ctx->block[63] = (uint8_t)(bits >> 0  & 0xff);
//     sha1_process_block(ctx);
//
//     {
//         Sha1Digest ret;
//         int i;
//         for (i = 0; i < 5; i++)
//             ret.digest[i] = sha1_get32((u8*)&ctx->h[i]);
//         sha1_reset(ctx);
//         return ret;
//     }
// }

// fn Sha1Digest sha1_compute(String bytes)
// {
//     Sha1Context ctx;
//     sha1_reset(&ctx);
//     sha1_write(&ctx, bytes);
//     return sha1_get_digest(&ctx);
// }

