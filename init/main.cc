#include <cstdio>
#include <string>

namespace trace {

class Trace
{
  private:
    std::string func_name_;
  public:
    Trace(const char *func)
      : func_name_ {func}
    {
        std::fprintf(stderr, "%s entered\n", func_name_.c_str());
    }

    ~Trace()
    {
        std::fprintf(stderr, "%s leaved\n", func_name_.c_str());
    }
};

} /* namespace */

#define TRACE do { trace::Trace{ __PRETTY_FUNCTION__ }; } while (0);

namespace demo {

class Int final
{
  public:
    Int( int value, const std::string& name = "temp");

    Int( const Int& other, const std::string& name = "temp");
    Int& operator=( const Int& other);

    ~Int();

    Int  operator+ ( const Int& other);
    Int& operator+=( const Int& other);

    int getId() const { return id_; }


  private:
    int value_;
    int id_;

    static int unique_id_;
    std::string name_;
};

int Int::unique_id_ = 0;

Int::Int( int value,
          const std::string& name)
  : value_{ value},
    id_{ unique_id_++},
    name_{ name}
{
    TRACE
    std::fprintf(stderr, "%d -> %s(%d) \n", value_, name_.c_str(), id_);
}

Int::Int( const Int& other, const std::string& name)
  : Int::Int{ other.value_, name}
{
    std::fprintf(stderr, "copy ctor %s(%d) = %s(%d) \n", name_.c_str(), id_, other.name_.c_str(), other.id_);
}

Int&
Int::operator=( const Int& other)
{
    TRACE
    std::fprintf(stderr, "assignment %s(%d) = %s(%d) \n", name_.c_str(), id_, other.name_.c_str(), other.id_);
    value_ = other.value_;
    return (*this);
}

Int&
Int::operator+=( const Int& other)
{
    TRACE
    std::fprintf(stderr, "%s(%d) += %s(%d) \n", name_.c_str(), id_, other.name_.c_str(), other.id_);
    value_ += other.value_;
    return (*this);
}

Int
Int::operator+( const Int& other)
{
    TRACE
    std::fprintf(stderr, "%s(%d) + %s(%d) \n", name_.c_str(), id_, other.name_.c_str(), other.id_);
    Int result{ *this};
    return (result += other);
}

Int::~Int()
{
    TRACE
}

} /* namespace demo */

int
main()
{
    demo::Int a{ 0, "a"};
    demo::Int b{ 0, "b"};
    demo::Int c{ 0, "c"};
    demo::Int d{ 0, "d"};

    c = a + 3;
    d = c + b;

    return 0;
}

