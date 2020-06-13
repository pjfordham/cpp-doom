#ifndef CRISPY_DOOM_MEMORY_HPP
#define CRISPY_DOOM_MEMORY_HPP

#include <new>
#include <cstdlib>
#include <cstring>
#include <type_traits>

#include "../src/z_zone.hpp"

#include <array>
#include <cstddef>

template<typename T, std::size_t size>
class circular_buffer {
   typename std::array<T, size> data;
   std::size_t start;
   class iterator : public std::iterator<std::forward_iterator_tag, int, std::size_t> {
      circular_buffer &cbuf;
      std::size_t index;
   public:
   typedef T value_type;
      explicit iterator( circular_buffer &_cbuf, std::size_t i ) : cbuf(_cbuf), index{i} {}
      iterator operator++() {
         index++;
         if ( index == size )
            index = 0;
         return *this;
      }
      iterator operator+(std::size_t val) {
         index = ( index + val ) % size;
         return *this;
      }
      bool operator==( iterator a ) {
         return a.index == index;
      }
      bool operator!=( iterator a ) {
         return a.index != index;
      }
      T & operator*() { return cbuf.data[ index ]; }
   };
   
public:
   typedef T value_type;
   circular_buffer() :  start( 0 ) {
   }
   
   iterator begin() {
      return iterator{*this,start};
   }
   iterator end() {
      return iterator{*this,size};
   }
   
   void push_back( const T& item ) {
      data[ start ] = item;
      start++;
      if ( start == size ) {
         start = 0;
      }
   }

   T& operator[]( std::size_t i ) {
      return data[ (start + i ) % size ];
   }
};

// todo fix me
template<typename DataType>
auto create_struct()
{
    auto *mem = malloc(sizeof(DataType));
   // The default {} here may be unnecessary we need to review all call-sites
    return new (mem) DataType{};
}

template <typename DataType> auto create_struct(const std::size_t size) {
  auto *mem = malloc(sizeof(DataType) * size);
  return static_cast<DataType *>(new (mem) DataType[size]);
}

template<typename DataType>
auto zone_calloc(int tag)
{
   auto *mem = Z_Malloc(sizeof(DataType), tag, nullptr);
   return new (mem) DataType{};
}

template<typename DataType>
auto zone_calloc(int tag, const std::size_t size)
{
   auto *mem = Z_Malloc(sizeof(DataType) * size, tag, nullptr);
   return new (mem) DataType[size]{};
}

// PTR is typically the pointer to the pointer that stores the Z_Malloc'd value
// and is used for statistics gathering, but it can be anything really.
template<typename DataType, typename PtrType>
auto zone_calloc(int tag, const std::size_t size, PtrType ptr)
{
   auto *mem = Z_Malloc(sizeof(DataType) * size, tag, static_cast<void*>(ptr));
   return new (mem) DataType[size]{};
}

#endif // CRISPY_DOOM_MEMORY_HPP
