#pragma once

#include <cstring>
#include <memory>
#include <algorithm>

int extra(unsigned N, unsigned E, unsigned k) {
  assert(E % N);
  u32 step = N - (E % N);
  return u64(step) * k % N;
}

bool isBigWord(unsigned N, unsigned E, unsigned k) {
  u32 step = N - (E % N); 
  return extra(N, E, k) + step < N;
  // return extra(N, E, k) < extra(N, E, k + 1);
}

u32 bitlen(int N, int E, int k) { return E / N + isBigWord(N, E, k); }

int &wordAt(int W, int H, int *data, int w) {
  int col  = w / 2 / H;
  int line = w / 2 % H;
  return data[(line * W + col) * 2 + w % 2];
}

std::vector<u32> compactBits(int *data, int W, int H, int E) {
  std::vector<u32> out;

  int carry = 0;
  u32 outWord = 0;
  int haveBits = 0;
  
  int N = 2 * W * H;
  for (int p = 0; p < N; ++p) {
    int w = wordAt(W, H, data, p) + carry;
    carry = 0;
    int bits = bitlen(N, E, p);
    while (w < 0) {
      w += 1 << bits;
      carry -= 1;
    }
    while (w >= (1 << bits)) {
      w -= 1 << bits;
      carry += 1;
    }
    assert(0 <= w && w < (1 << bits));
    while (bits) {
      assert(haveBits < 32);
      outWord |= w << haveBits;
      if (haveBits + bits >= 32) {
        w >>= (32 - haveBits);
        bits -= (32 - haveBits);
        out.push_back(outWord);
        outWord = 0;
        haveBits = 0;
      } else {
        haveBits += bits;
        bits = 0;
      }
    }
  }
  if (haveBits) {
    out.push_back(outWord);
    haveBits = 0;
  }

  for (int p = 0; carry; ++p) {
    i64 v = i64(out[p]) + carry;
    out[p] = v & 0xffffffff;
    carry = v >> 32;
  }

  assert(int(out.size()) == (E - 1) / 32 + 1);
  return out;
}

void expandBits(const std::vector<u32> &compactBits, bool balanced, int W, int H, int E, int *data) {
  // This is similar to carry propagation.
  int N = 2 * W * H;
  int haveBits = 0;
  u64 bits = 0;

  assert(E % 32 != 0);
  auto it = compactBits.cbegin(), itEnd = compactBits.cend();
  for (int p = 0; p < N; ++p) {
    int len = bitlen(N, E, p);
    if (haveBits < len) {
      assert(it != itEnd);
      bits += u64(*it++) << haveBits;
      haveBits += 32;
    }
    assert(haveBits >= len);
    int b = bits & ((1 << len) - 1);
    bits >>= len;
    if (balanced) {
      // turn the (len - 1) bit of b into sign bit.
      b = (b << (32 - len)) >> (32 - len);
      if (b < 0) { ++bits; }
    }
    wordAt(W, H, data, p) = b;
    // bits = (bits - b) >> len;
    haveBits -= len;
  }
  assert(it == itEnd);
  assert(haveBits == 32 - E % 32);
  assert(!bits || (balanced && (bits == 1)));
  data[0] += bits;
}
