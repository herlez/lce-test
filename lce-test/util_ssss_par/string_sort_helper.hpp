#pragma once
#include <algorithm>
#include "ssss_par.hpp"
namespace lce_test::par {

struct mock_string {
  using size_type = uint64_t;

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
                [[maybe_unused]]const typename Traits::String& b,
                const typename Traits::CharIterator& bi) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    //After scanning 3*tau character, suffixes  are ordered by run_info and then start position
    //This guarantees that they are not equal
    if(ss.is_exact_end(a, ai)) { return false; }
    return (*ai == *bi);
  }

  //! check if string a is less or equal to string b at iterators ai and bi.
  bool is_less(const typename Traits::String& a,
               const typename Traits::CharIterator& ai,
               const typename Traits::String& b,
               const typename Traits::CharIterator& bi) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    if(ss.is_exact_end(a, ai)) {
      if(ss.is_less_run(a, b)) { return true; }
      return a > b;
    }
    return (*ai < *bi);
  }

  //! check if string a is less or equal to string b at iterators ai and bi.
  bool is_leq(const typename Traits::String& a,
              const typename Traits::CharIterator& ai,
              const typename Traits::String& b,
              const typename Traits::CharIterator& bi) const {
    const StringSet& ss = *static_cast<const StringSet*>(this);
    //if(ss.has_runs() && ss.is_exact_end(a, ai) && ss.is_exact_end(b, bi)) { return ss.is_leq_run(a, b); }
    if(ss.is_exact_end(a, ai)) { 
      //if(ss.is_leq_run(a, b)) { return true; }
      if(ss.is_less_run(a, b)) { return true; }
      return a > b;
    }
    return (*ai <= *bi);
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
template<typename sss_type = uint32_t>
class StringShortSuffixSetTraits {
 public:
  //! exported alias for assumed string container
  typedef mock_string Text;

  //! exported alias for character type
  typedef uint8_t Char;

  //! String reference: suffix index of the text.
  typedef sss_type String;

  //! Iterator over string references: using std::vector's iterator over
  //! suffix array vector
  typedef typename std::vector<String>::iterator Iterator;

  //! iterator of characters in a string
  typedef const Char* CharIterator;
};

/*!
 * Class implementing StringSet concept for suffix sorting indexes of a
 * std::string text object.
 */
template <uint64_t t_tau, typename sss_type>
class StringShortSuffixSet
    : public StringShortSuffixSetTraits<sss_type>,
      public StringSetBase<StringShortSuffixSet<t_tau, sss_type>, StringShortSuffixSetTraits<sss_type>> {

 public:
  //! exported alias for assumed string container
  typedef typename StringShortSuffixSetTraits<sss_type>::Text Text;
  //! exported alias for character type
  typedef typename StringShortSuffixSetTraits<sss_type>::Char Char;
  //! String reference: suffix index of the text.
  typedef typename StringShortSuffixSetTraits<sss_type>::String String;
  //! Iterator over string references: using std::vector's iterator over
  //! suffix array vector
  typedef typename StringShortSuffixSetTraits<sss_type>::Iterator Iterator;
  //! exported alias for assumed string container
  typedef std::tuple<Text, std::vector<String>, string_synchronizing_set_par<t_tau/3, sss_type> const&> Container;
  //! iterator of characters in a string
  typedef typename StringShortSuffixSetTraits<sss_type>::CharIterator CharIterator;

  //! Construct from begin and end string pointers
  StringShortSuffixSet(const Text& text,
                       const Iterator& begin, const Iterator& end, string_synchronizing_set_par<t_tau/3, sss_type> const& sss)
      : text_(&text),
        begin_(begin),
        end_(end),
        sss_(sss),
        sss_has_runs_(sss.has_runs()) {}

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
  bool is_end([[maybe_unused]]const String& str, [[maybe_unused]]const CharIterator& i) const {
    //We order equal suffixes by their starting index.
    //Because of that we don't need an end.
    return false;
  }
  //! Returns true if CharIterator is at the exact end of the given String
  bool is_exact_end(const String& str, const CharIterator i) const {
    return i == reinterpret_cast<CharIterator>(std::min(text_->data() + str + t_tau, text_->data() + text_->size()));
  } //TODO: i == ... seems wrong; should be i >= ...

  //if(ss.has_runs() && ss.is_exact_end(a, ai)) { return ss.is_smaller_run(a, b) };
  bool has_runs() const {
    return sss_has_runs_;
  }  

  bool is_less_run(const String& a, const String& b) const {
    int64_t run_info_a = sss_.get_run_info(a);
    int64_t run_info_b = sss_.get_run_info(b);
    return run_info_a < run_info_b;
  }

  bool is_equal_run(const String& a, const String& b) const {
    int64_t run_info_a = sss_.get_run_info(a);
    int64_t run_info_b = sss_.get_run_info(b);
    return run_info_a == run_info_b;
  }

  bool is_leq_run(const String& a, const String& b) const {
    int64_t run_info_a = sss_.get_run_info(a);
    int64_t run_info_b = sss_.get_run_info(b);
    return run_info_a <= run_info_b;
  }
  //! Return complete string (for debugging purposes)
  //std::string get_string(const String& s, size_t depth = 0) const { return text_->substr(s + depth); }

  //! Subset this string set using iterator range.
  StringShortSuffixSet sub(Iterator begin, Iterator end) const { return StringShortSuffixSet(*text_, begin, end, sss_); }

  //! Allocate a new temporary string container with n empty Strings
  Container allocate(size_t n) const { return std::make_tuple(*text_, std::vector<String>(n), sss_); }

  //! Deallocate a temporary string container
  static void deallocate(Container& c) {
    std::vector<String> v;
    v.swap(std::get<1>(c));
  }

  //! Construct from a string container
  explicit StringShortSuffixSet(Container& c)
      : text_(&std::get<0>(c)),  
        begin_(std::get<1>(c).begin()),
        end_(std::get<1>(c).end()),
        sss_(std::get<2>(c)),
        sss_has_runs_(std::get<2>(c).has_runs()) {}

 protected:
  //! reference to base text
  const Text* text_;

  //! iterators inside the output suffix array.
  Iterator begin_, end_;
  string_synchronizing_set_par<t_tau/3, sss_type> const& sss_;
  bool sss_has_runs_;
};
}  // namespace lce_test::par