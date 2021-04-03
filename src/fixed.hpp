//
// Fixed point, 32bit as 16.16.
//

// #include <fmt/core.h>
#include <type_traits>


template <typename Integer, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>>
class ffixed_t {
public:
   Integer value;
   static int precision() { return Precision; }
   explicit operator bool() const { return value != 0; }
   ffixed_t() = default;
   explicit ffixed_t(Integer _value) : value{ _value } {   }
   explicit operator Integer() const { return value; }

   ffixed_t abs() {
      return value < 0 ? ffixed_t(-value) : ffixed_t(value);
   }
};

template <typename Integer>
class ffixed_t<Integer, 0> {
public:
   Integer value;
   static int precision() { return 0; }
   ffixed_t() = default;
   explicit operator bool() const { return value != 0; }

   ffixed_t(Integer _value) : value{ _value } {   }
   operator Integer() const { return value; }

   ffixed_t abs() {
      return value < 0 ? ffixed_t(-value) : ffixed_t(value);
   }
};

// template <typename Integer, int Precision>
// struct fmt::formatter<ffixed_t<Integer,Precision>> {
//    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

//    template <typename FormatContext>
//    auto format(const ffixed_t<Integer,Precision>& t, FormatContext& ctx) {
//       return format_to(ctx.out(), "{} {}", t.value, t.precision );
//    }
// };

// Well defined operators

// comparison between two fixed types with same precision but different base types
template <typename Integer, typename Integer2, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
bool operator<(ffixed_t<Integer,Precision> lhs, ffixed_t<Integer2,Precision> rhs) {
   return lhs.value < rhs.value;
}
template <typename Integer, typename Integer2, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
bool operator>(ffixed_t<Integer,Precision> lhs, ffixed_t<Integer2,Precision> rhs) {
   return lhs.value > rhs.value;
}
template <typename Integer, typename Integer2,int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
bool operator==(ffixed_t<Integer,Precision> lhs, ffixed_t<Integer2,Precision> rhs) {
   return lhs.value == rhs.value;
}
template <typename Integer,  typename Integer2, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
bool operator<=(ffixed_t<Integer,Precision> lhs, ffixed_t<Integer2,Precision> rhs) {
   return lhs.value <= rhs.value;
}
template <typename Integer, typename Integer2, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
bool operator>=(ffixed_t<Integer,Precision> lhs, ffixed_t<Integer2,Precision> rhs) {
   return lhs.value >= rhs.value;
}

// unary negation of any combination
template <typename Integer, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>>
ffixed_t<Integer,Precision> operator-(ffixed_t<Integer,Precision> lhs) {
   return ffixed_t<Integer,Precision>(-lhs.value);
}

// binary operators, fixed_t, fixed_t => fixed_t, same Precisio different base type
template <typename Integer, typename Integer2, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
auto operator+(ffixed_t<Integer,Precision> lhs, ffixed_t<Integer2,Precision> rhs) {
   return ffixed_t<decltype(lhs.value + rhs.value), Precision>(lhs.value + rhs.value);
}
template <typename Integer,typename Integer2, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
auto operator-(ffixed_t<Integer,Precision> lhs, ffixed_t<Integer2,Precision> rhs) {
   return ffixed_t<decltype(lhs.value - rhs.value), Precision>(lhs.value - rhs.value);
}
template <typename Integer, typename Integer2, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
auto operator^(ffixed_t<Integer,Precision> lhs, ffixed_t<Integer2,Precision> rhs) {
   return ffixed_t<decltype(lhs.value ^ rhs.value), Precision>(lhs.value ^ rhs.value);
}
template <typename Integer, typename Integer2, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
auto operator&(ffixed_t<Integer,Precision> lhs, ffixed_t<Integer2,Precision> rhs) {
   return ffixed_t<decltype(lhs.value & rhs.value), Precision>(lhs.value & rhs.value);
}
template <typename Integer, typename Integer2, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>
          >
auto operator%(ffixed_t<Integer,Precision> lhs, ffixed_t<Integer2,Precision> rhs) {
   return ffixed_t<decltype(lhs.value % rhs.value),Precision>(lhs.value % rhs.value);
}


// binary, fixed_t, int => fixed_t, fixed_t shifted left or right by an int
template <typename Integer, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>>
ffixed_t<Integer,Precision> operator<<(ffixed_t<Integer,Precision> lhs, int rhs) {
   return ffixed_t<Integer,Precision>(lhs.value << rhs);
}
template <typename Integer, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>>
ffixed_t<Integer,Precision> operator>>(ffixed_t<Integer,Precision> lhs, int rhs) {
   return ffixed_t<Integer,Precision>(lhs.value >> rhs);
}

// updating versions of above, += -= *= >>= <<=
template <typename Integer, typename Integer2, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
ffixed_t<Integer,Precision> operator+=(ffixed_t<Integer,Precision> &lhs, ffixed_t<Integer2,Precision> rhs) {
   return ffixed_t<Integer,Precision>(lhs.value += rhs.value);
}
template <typename Integer, typename Integer2,int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
ffixed_t<Integer,Precision> operator-=(ffixed_t<Integer,Precision> &lhs, ffixed_t<Integer2,Precision> rhs) {
   return ffixed_t<Integer,Precision>(lhs.value -= rhs.value);
}
template <typename Integer, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>
          >
ffixed_t<Integer,Precision> operator*=(ffixed_t<Integer,Precision> &lhs, int rhs) {
   return ffixed_t<Integer,Precision>(lhs.value *= rhs);
}
template <typename Integer, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>
          >
ffixed_t<Integer,Precision> operator>>=(ffixed_t<Integer,Precision> &lhs, int rhs) {
   return ffixed_t<Integer,Precision>(lhs.value >>= rhs);
}
template <typename Integer, int Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>
          >
ffixed_t<Integer,Precision> operator<<=(ffixed_t<Integer,Precision> &lhs, int rhs) {
   return ffixed_t<Integer,Precision>(lhs.value <<= rhs);
}

// binary, fixed_t, fixed_t => other fixed_t muliplt and divide with scaled precision but same base type 
template <typename Integer, typename Integer2, int LHS_Precision, int RHS_Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>
          >
auto operator*(ffixed_t<Integer,LHS_Precision> lhs, ffixed_t<Integer2,RHS_Precision> rhs) {
   return ffixed_t<decltype(lhs.value * rhs.value), LHS_Precision+RHS_Precision>(lhs.value * rhs.value);
}
template <typename Integer, typename Integer2, int LHS_Precision, int RHS_Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>
          >
auto operator/(ffixed_t<Integer,LHS_Precision> lhs, ffixed_t<Integer2,RHS_Precision> rhs) {
   return ffixed_t<decltype(lhs.value / rhs.value), LHS_Precision-RHS_Precision>(lhs.value / rhs.value);
}

// binary, multiple and divide by int
template <typename Integer, typename Integer2, int LHS_Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>
          >
auto operator*(ffixed_t<Integer,LHS_Precision> lhs, Integer2 rhs) {
   return ffixed_t<decltype(lhs.value * rhs), LHS_Precision>(lhs.value * rhs);
}

template <typename Integer, typename Integer2, int RHS_Precision,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>,
         typename = std::enable_if_t<std::is_integral_v<Integer>, bool>
          >
auto operator*(Integer lhs, ffixed_t<Integer2,RHS_Precision> rhs) {
   return ffixed_t<decltype(lhs * rhs.value), RHS_Precision>(lhs * rhs.value);
}

template <typename Integer, int LHS_Precision,  typename Integer2,
          typename = std::enable_if_t<std::is_integral_v<Integer>, bool>,
          typename = std::enable_if_t<std::is_integral_v<Integer2>, bool>>
auto operator/(ffixed_t<Integer,LHS_Precision> lhs, Integer2 rhs) {
   return ffixed_t<decltype(lhs.value / rhs), LHS_Precision>(lhs.value / rhs);
}
