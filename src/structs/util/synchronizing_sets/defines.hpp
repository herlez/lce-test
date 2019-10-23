// Code from the TLX, as this is used in the original implementation

#if defined(__GNUC__) || defined(__clang__)
#define TLX_LIKELY(c)   __builtin_expect((c), 1)
#define TLX_UNLIKELY(c) __builtin_expect((c), 0)
#else
#define TLX_LIKELY(c)   c
#define TLX_UNLIKELY(c) c
#endif

#if defined(__GNUC__) || defined(__clang__)
#define TLX_ATTRIBUTE_PACKED __attribute__ ((packed))
#else
#define TLX_ATTRIBUTE_PACKED
#endif
