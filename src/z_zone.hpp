//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//	Remark: this was the only stuff that, according
//	 to John Carmack, might have been useful for
//	 Quake.
//



#ifndef __Z_ZONE__
#define __Z_ZONE__

#include <stdio.h>
#include <type_traits>

//
// ZONE MEMORY
// PU - purge tags.

struct memblock_t
{
   typedef void (*destructor_t) ( void *mem, std::size_t count);
   int			size;	// including the header and possibly tiny fragments
   void**		user;
   int			tag;	// PU_FREE if this is free
   int			id;	// should be ZONEID
   destructor_t destructor;
   int                  count;
   struct memblock_t*	next;
   struct memblock_t*	prev;
};

enum
{
    PU_STATIC = 1,                  // static entire execution time
    PU_SOUND,                       // static while playing
    PU_MUSIC,                       // static while playing
    PU_FREE,                        // a free block
    PU_LEVEL,                       // static until level exited
    PU_LEVSPEC,                     // a special thinker in a level
    
    // Tags >= PU_PURGELEVEL are purgable whenever needed.

    PU_PURGELEVEL,
    PU_CACHE,

    // Total number of different tag types

    PU_NUM_TAGS
};


void *Z_Malloc(int size, int tag, void *user, memblock_t::destructor_t = nullptr, int count = 1);

template <typename Type>
Type *Z_New( int tag, int count = 1, Type **user = nullptr) {
   // Setup the destructor lambda
   auto destructor = std::is_trivially_destructible<Type>::value ? nullptr :
         [] ( void *mem, std::size_t count ) {
         auto ptr = static_cast<Type*>(mem);
         for (int i = 0; i < count ;i++ ) {
            (ptr + i)->~Type();
         }
      };

   auto ptr = static_cast<Type*>( Z_Malloc( sizeof(Type) * count, tag,
                                            user, destructor, count ) );

   // Run the constructors
   if (!std::is_trivially_constructible<Type>::value) {
      for (int i = 0; i < count ;i++ ) {
         new (ptr + i) Type;
      }
   }
   return ptr;

}

void    Z_Free (void *ptr, bool run_destructor = false);

template<typename Type>
void Z_Delete( Type *mem ) {

   // Run the destructor directly
   bool run_destructor = !std::is_trivially_destructible<Type>::value;

   // FIXME we know the type here so we don't need to call thru the lambda

   // Free the underlying memory
   Z_Free(mem, run_destructor);
}

void	Z_Init (void);
void    Z_FreeTags (int lowtag, int hightag);
void    Z_DumpHeap (int lowtag, int hightag);
void    Z_FileDumpHeap (FILE *f);
void    Z_CheckHeap (void);
void    Z_ChangeTag2 (void *ptr, int tag, const char *file, int line);
void    Z_ChangeUser(void *ptr, void **user);
int     Z_FreeMemory (void);
unsigned int Z_ZoneSize(void);

//
// This is used to get the local FILE:LINE info from CPP
// prior to really call the function in question.
//
#define Z_ChangeTag(p,t)                                       \
    Z_ChangeTag2((p), (t), __FILE__, __LINE__)


#endif
