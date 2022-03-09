#pragma once
#include <assert.h>

#include <chrono>
#include <iterator>
#include <random>
#include <bit>


namespace herlez::rolling_hash {
__extension__ typedef unsigned __int128 uint128_t;

inline std::ostream &operator<<(std::ostream &out, uint128_t value) {
  std::ostream::sentry s(out);
  if (s) {
    char buffer[64];  // 39 should be enough
    char *digit = &(buffer[64]);
    // /*
    unsigned int width = std::bit_width(value);
    *(--digit) = ')';
    do {
      *(--digit) = "0123456789"[width % 10];
      width /= 10;
    } while (width != 0);
    *(--digit) = '(';
    // */
    do {
      *(--digit) = "0123456789"[value % 10];
      value /= 10;
    } while (value != 0);
    int len = &(buffer[64]) - digit;
    if (out.rdbuf()->sputn(digit, len) != len) {
      out.setstate(std::ios_base::badbit);
    }
  }
  return out;
}

template <typename t_it, size_t t_prime_exp = 107>
class rk_prime {
 public:
  rk_prime(t_it fp_begin, uint128_t tau, uint128_t base = 0)
      : m_tau(tau),
        m_fp_begin(fp_begin),
        m_fp_end(fp_begin + m_tau),
        m_cur_fp(0) {
    if (base == 0) {
      uint64_t max = (t_prime_exp > 64) ? ((uint64_t{1} << (127 - std::bit_width(m_prime))) - 1) : m_prime;
      m_base = static_cast<uint128_t>(random64(1, max));
    } else {
      m_base = base;
    }
    //prime should be mersenne and (prime*base + prime) should not overflow
    //static_assert(t_prime_exp == 107 || t_prime_exp == 61 || t_prime_exp == 89);
    //std::cout << m_prime << " + " << m_base << '\n';
    assert(std::bit_width(m_prime) + std::bit_width(m_base) <= 127);

    fillPowerTable();
    //Calculate first window
    for (size_t i = 0; i < m_tau; ++i) {
      m_cur_fp *= m_base;
      m_cur_fp += (unsigned char)m_fp_begin[i];
      m_cur_fp = mod_m_prime(m_cur_fp);
    }
  }

  inline uint128_t roll() {
    m_cur_fp *= m_base;
    uint128_t border_char_influence = m_char_influence[(unsigned char)(*m_fp_begin)][(unsigned char)(*m_fp_end)];
    m_cur_fp += border_char_influence;
    m_cur_fp = mod_m_prime(m_cur_fp);

    std::advance(m_fp_begin, 1);
    std::advance(m_fp_end, 1);
    return m_cur_fp;
  }

  inline uint128_t get_current_fp() const {
    return m_cur_fp;
  }

  inline uint128_t get_base() const {
    return m_base;
  }

 private:
  static constexpr uint128_t m_prime = (uint128_t{1} << t_prime_exp) - 1;
  uint128_t m_tau;
  t_it m_fp_begin;
  t_it m_fp_end;
  uint128_t m_cur_fp;

  uint128_t m_base;
  uint128_t m_char_influence[256][256];

  inline static uint64_t random64(uint64_t min, uint64_t max) {
    static std::mt19937_64 g = std::mt19937_64(std::random_device()());
    return (std::uniform_int_distribution<uint64_t>(min, max))(g);
  }

  inline uint128_t mod_m_prime(uint128_t num) const {
    //Does only work for 2^127 - 1
    // uint128_t const z = (num + 1) >> t_prime_exp;
    // return (num + z) & m_prime;

    // uint128_t const v = num + 1;
    // uint64_t const z = ((v >> t_prime_exp) + v) >> t_prime_exp;
    // return (num + z) & m_prime;

    num = (num & m_prime) + (num >> t_prime_exp);
    return (num >= m_prime) ? (num - m_prime) : num;
  }

  // To compute (a * b) % mod
  uint128_t mulmod(uint128_t a, uint128_t b) const {
    uint128_t res = 0;  // Initialize result
    a = a % m_prime;
    while (b > 0) {
      // If b is odd, add 'a' to result
      if (b % 2 == 1)
        res = (res + a) % m_prime;

      // Multiply 'a' with 2
      a = (a * 2) % m_prime;

      // Divide b by 2
      b /= 2;
    }
    // Return result
    return res % m_prime;
  }

  uint128_t calculatePowerModulo() const {
    uint128_t result = 1;
    uint128_t b = m_base;
    uint128_t exponent = m_tau;
    while (exponent > 0) {
      if (exponent & 1ULL) result = mulmod(b, result);
      b = mulmod(b, b);
      exponent >>= 1;
    }
    return result;
  }

  void fillPowerTable() {
    //std::chrono::system_clock::time_point begin_ = std::chrono::system_clock::now();
    const uint128_t m_two_pow_tau_mod_q = calculatePowerModulo();
    for (size_t i = 0; i < 256; ++i) {
      m_char_influence[i][0] = m_prime - (mod_m_prime(m_two_pow_tau_mod_q * i));
      for (size_t j = 1; j < 256; ++j) {
        m_char_influence[i][j] = mod_m_prime(m_char_influence[i][j - 1] + 1);
      }
    }
    //std::chrono::system_clock::time_point const end = std::chrono::system_clock::now();
    //std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - begin_).count() << '\n';
  }
};
}  // namespace herlez::rolling_hash