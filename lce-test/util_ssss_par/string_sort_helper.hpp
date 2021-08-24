#pragma once
#include <algorithm>

namespace lce_test::par {

struct mock_string {
  using size_type = uint32_t;

  mock_string(uint8_t const* const str, size_type size) : m_str(str), m_size(size) {}

  size_type size() const {
    return m_size;
  }
  uint8_t const* data() const {
    return m_str;
  }

 private:
  uint8_t const* const m_str;
  size_type m_size;
};

template <typename StringSet, typename Traits>
class StringSetBase {
 public:
  //! index-based array access (readable and writable) to String objects.
  typename Traits::String& at(size_t i) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    return *(ss.begin() + i);
  }

  //! \name CharIterator Comparisons
  //! \{

  //! check equality of two strings a and b at char iterators ai and bi.
  bool is_equal(const typename Traits::String& a,
                const typename Traits::CharIterator& ai,
                const typename Traits::String& b,
                const typename Traits::CharIterator& bi) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    return !ss.is_end(a, ai) && !ss.is_end(b, bi) && (*ai == *bi);
  }

  //! check if string a is less or equal to string b at iterators ai and bi.
  bool is_less(const typename Traits::String& a,
               const typename Traits::CharIterator& ai,
               const typename Traits::String& b,
               const typename Traits::CharIterator& bi) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    return ss.is_end(a, ai) ||
           (!ss.is_end(a, ai) && !ss.is_end(b, bi) && *ai < *bi);
  }

  //! check if string a is less or equal to string b at iterators ai and bi.
  bool is_leq(const typename Traits::String& a,
              const typename Traits::CharIterator& ai,
              const typename Traits::String& b,
              const typename Traits::CharIterator& bi) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    return ss.is_end(a, ai) ||
           (!ss.is_end(a, ai) && !ss.is_end(b, bi) && *ai <= *bi);
  }

  //! \}

  //! \name Character Extractors
  //! \{

  typename Traits::Char
  get_char(const typename Traits::String& s, size_t depth) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    return *ss.get_chars(s, depth);
  }

  //! Return up to 1 characters of string s at iterator i packed into a
  //! uint8_t (only works correctly for 8-bit characters)
  uint8_t get_uint8(
      const typename Traits::String& s, typename Traits::CharIterator i) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);

    if (ss.is_end(s, i)) return 0;
    return uint8_t(*i);
  }

  //! Return up to 2 characters of string s at iterator i packed into a
  //! uint16_t (only works correctly for 8-bit characters)
  uint16_t get_uint16(
      const typename Traits::String& s, typename Traits::CharIterator i) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);

    uint16_t v = 0;
    if (ss.is_end(s, i)) return v;
    v = (uint16_t(*i) << 8);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint16_t(*i) << 0);
    return v;
  }

  //! Return up to 4 characters of string s at iterator i packed into a
  //! uint32_t (only works correctly for 8-bit characters)
  uint32_t get_uint32(
      const typename Traits::String& s, typename Traits::CharIterator i) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);

    uint32_t v = 0;
    if (ss.is_end(s, i)) return v;
    v = (uint32_t(*i) << 24);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint32_t(*i) << 16);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint32_t(*i) << 8);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint32_t(*i) << 0);
    return v;
  }

  //! Return up to 8 characters of string s at iterator i packed into a
  //! uint64_t (only works correctly for 8-bit characters)
  uint64_t get_uint64(
      const typename Traits::String& s, typename Traits::CharIterator i) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);

    uint64_t v = 0;
    if (ss.is_end(s, i)) return v;
    v = (uint64_t(*i) << 56);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint64_t(*i) << 48);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint64_t(*i) << 40);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint64_t(*i) << 32);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint64_t(*i) << 24);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint64_t(*i) << 16);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint64_t(*i) << 8);
    ++i;
    if (ss.is_end(s, i)) return v;
    v |= (uint64_t(*i) << 0);
    return v;
  }

  uint8_t get_uint8(const typename Traits::String& s, size_t depth) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    return get_uint8(s, ss.get_chars(s, depth));
  }

  uint16_t get_uint16(const typename Traits::String& s, size_t depth) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    return get_uint16(s, ss.get_chars(s, depth));
  }

  uint32_t get_uint32(const typename Traits::String& s, size_t depth) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    return get_uint32(s, ss.get_chars(s, depth));
  }

  uint64_t get_uint64(const typename Traits::String& s, size_t depth) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    return get_uint64(s, ss.get_chars(s, depth));
  }

  //! \}

  //! Subset this string set using index range.
  StringSet subi(size_t begin, size_t end) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    return ss.sub(ss.begin() + begin, ss.begin() + end);
  }

  bool check_order(const typename Traits::String& s1,
                   const typename Traits::String& s2) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);

    typename StringSet::CharIterator c1 = ss.get_chars(s1, 0);
    typename StringSet::CharIterator c2 = ss.get_chars(s2, 0);

    while (ss.is_equal(s1, c1, s2, c2))
      ++c1, ++c2;

    return ss.is_leq(s1, c1, s2, c2);
  }

  bool check_order() const {
    const StringSet& ss = *static_cast<const StringSet*>(this);

    for (typename Traits::Iterator pi = ss.begin();
         pi + 1 != ss.end(); ++pi) {
      if (!check_order(*pi, *(pi + 1))) {
        TLX_LOG1 << "check_order() failed at position " << pi - ss.begin();
        return false;
      }
    }
    return true;
  }

  void print() const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    size_t i = 0;
    for (typename Traits::Iterator pi = ss.begin(); pi != ss.end(); ++pi) {
      TLX_LOG1 << "[" << i++ << "] = " << *pi
               << " = " << ss.get_string(*pi, 0);
    }
  }
};

/*!
 * Class implementing StringSet concept for suffix sorting indexes of a
 * std::string text object.
 */
class StringShortSuffixSetTraits {
 public:
  //! exported alias for assumed string container
  typedef mock_string Text;

  //! exported alias for character type
  typedef uint8_t Char;

  //! String reference: suffix index of the text.
  typedef typename Text::size_type String;

  //! Iterator over string references: using std::vector's iterator over
  //! suffix array vector
  typedef typename std::vector<String>::iterator Iterator;

  //! iterator of characters in a string
  typedef const Char* CharIterator;

  //! exported alias for assumed string container
  typedef std::pair<Text, std::vector<String>> Container;
};

/*!
 * Class implementing StringSet concept for suffix sorting indexes of a
 * std::string text object.
 */
template <uint32_t t_tau>
class StringShortSuffixSet
    : public StringShortSuffixSetTraits,
      public StringSetBase<StringShortSuffixSet<t_tau>, StringShortSuffixSetTraits> {
 public:
  //! Construct from begin and end string pointers
  StringShortSuffixSet(const Text& text,
                       const Iterator& begin, const Iterator& end)
      : text_(&text),
        begin_(begin),
        end_(end) {}

  //! Initializing constructor which fills output vector sa with indices.

  //! Return size of string array
  size_t size() const { return end_ - begin_; }
  //! Iterator representing first String position
  Iterator begin() const { return begin_; }
  //! Iterator representing beyond last String position
  Iterator end() const { return end_; }

  //! Array access (readable and writable) to String objects.
  String& operator[](const Iterator& i) const { return *i; }

  //! Return CharIterator for referenced string, which belongs to this set.
  CharIterator get_chars(const String& s, size_t depth) const { return reinterpret_cast<CharIterator>(text_->data()) + s + depth; }

  //! Returns true if CharIterator is at end of the given String
  bool is_end(const String& str, const CharIterator& i) const {
    return i >= reinterpret_cast<CharIterator>(std::min(text_->data() + str + t_tau, text_->data() + text_->size()));
  }

  //! Return complete string (for debugging purposes)
  //std::string get_string(const String& s, size_t depth = 0) const { return text_->substr(s + depth); }

  //! Subset this string set using iterator range.
  StringShortSuffixSet sub(Iterator begin, Iterator end) const { return StringShortSuffixSet(*text_, begin, end); }

  //! Allocate a new temporary string container with n empty Strings
  Container allocate(size_t n) const { return std::make_pair(*text_, std::vector<String>(n)); }

  //! Deallocate a temporary string container
  static void deallocate(Container& c) {
    std::vector<String> v;
    v.swap(c.second);
  }

  //! Construct from a string container
  explicit StringShortSuffixSet(Container& c)
      : text_(&c.first),
        begin_(c.second.begin()),
        end_(c.second.end()) {}

 protected:
  //! reference to base text
  const Text* text_;

  //! iterators inside the output suffix array.
  Iterator begin_, end_;
};
}  // namespace lce_test::par