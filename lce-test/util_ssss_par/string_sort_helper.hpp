#pragma once


namespace lce_test::par {

class MockString
{
  const char* const m_ptr;
  const int m_size;

  public:
  MockString(const char* const ptr, int size) : m_ptr(ptr), m_size(size) {
  }

  size_t size() const {
    return m_size;
  }

  MockString substr(size_t i) const {
    return MockString(m_ptr + i, m_size - i);
  }

  const char* data() const {
    return m_ptr;
  }
};

/*!
 * Class implementing StringSet concept for suffix sorting indexes of a
 * std::string text object.
 */
class StringShortSuffixSetTraits
{
public:
    //! exported alias for assumed string container
    typedef std::string Text;

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
    typedef std::pair<MockString, std::vector<String> > Container;
};

template<uint64_t t_length>
class StringShortSuffixSet
    : public StringShortSuffixSetTraits,
      public tlx::sort_strings_detail::StringSetBase<StringShortSuffixSet<t_length>, tlx::sort_strings_detail::StringSuffixSetTraits>
{
public:
    //! Construct from begin and end string pointers
    StringShortSuffixSet(const MockString& text,
                    const Iterator& begin, const Iterator& end)
        : text_(&text),
          begin_(begin), end_(end)
    { }

    //! Initializing constructor which fills output vector sa with indices.
    static StringShortSuffixSet
    Initialize(const MockString& text, std::vector<String>& sa) {
        sa.resize(text.size());
        for (size_t i = 0; i < text.size(); ++i)
            sa[i] = i;
        return StringShortSuffixSet<t_length>(text, sa.begin(), sa.end());
    }

    //! Return size of string array
    size_t size() const { return end_ - begin_; }
    //! Iterator representing first String position
    Iterator begin() const { return begin_; }
    //! Iterator representing beyond last String position
    Iterator end() const { return end_; }

    //! Array access (readable and writable) to String objects.
    String& operator [] (const Iterator& i) const
    { return *i; }

    //! Return CharIterator for referenced string, which belongs to this set.
    CharIterator get_chars(const String& s, size_t depth) const
    { return reinterpret_cast<CharIterator>(text_->data()) + s + depth; }

    //! Returns true if CharIterator is at end of the given String
    bool is_end(const String& pos, const CharIterator& i) const
    { return i >= reinterpret_cast<CharIterator>(std::min(text_->data() + pos + t_length, text_->data() + text_->size()));}
    //{ return (i >= reinterpret_cast<CharIterator>(text_->data()) + text_->size()); }
    
    //! Return complete string (for debugging purposes)
    //std::string get_string(const String& s, size_t depth = 0) const
    //{ return text_->substr(s + depth); }

    //! Subset this string set using iterator range.
    StringShortSuffixSet<t_length> sub(Iterator begin, Iterator end) const
    { return StringShortSuffixSet(*text_, begin, end); }

    //! Allocate a new temporary string container with n empty Strings
    Container allocate(size_t n) const
    { return std::make_pair(*text_, std::vector<String>(n)); }

    //! Deallocate a temporary string container
    static void deallocate(Container& c)
    { std::vector<String> v; v.swap(c.second); }

    //! Construct from a string container
    explicit StringShortSuffixSet(Container& c)
        : text_(&c.first),
          begin_(c.second.begin()), end_(c.second.end())
    { }

protected:
    //! reference to base text
    const MockString* text_;

    //! iterators inside the output suffix array.
    Iterator begin_, end_;
};
}