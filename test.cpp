/*-
 * Provided to compare the default allocator to MemoryPool
 *
 * Check out StackAlloc.h for a stack implementation that takes an allocator as
 * a template argument. This may give you some idea about how to use MemoryPool.
 *
 * This code basically creates two stacks: one using the default allocator and
 * one using the MemoryPool allocator. It pushes a bunch of objects in them and
 * then pops them out. We repeat the process several times and time how long
 * this takes for each of the stacks.
 *
 * Do not forget to turn on optimizations (use -O2 or -O3 for GCC). This is a
 * benchmark, we want inlined code.
 */

#include <iostream>
#include <cassert>
#include <time.h>
#include <vector>

#include "MemoryPool.h"
#include "StackAlloc.h"

/* Adjust these values depending on how much you trust your computer */
#define ELEMS 1000000
#define REPS 50

int main()
{
  clock_t start;

  std::cout << "Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com\n";
  std::cout << "Provided to compare the default allocator to MemoryPool.\n\n";

  /* Use the default allocator */
  StackAlloc<int, std::allocator<int> > stackDefault;
  start = clock();
  for (int j = 0; j < REPS; j++)
  {
    assert(stackDefault.empty());
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackDefault.push(i);
      stackDefault.push(i);
      stackDefault.push(i);
      stackDefault.push(i);
    }
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackDefault.pop();
      stackDefault.pop();
      stackDefault.pop();
      stackDefault.pop();
    }
  }
  std::cout << "Default Allocator Time: ";
  std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

  /* Use MemoryPool */
  StackAlloc<int, MemoryPool<int> > stackPool;
  start = clock();
  for (int j = 0; j < REPS; j++)
  {
    assert(stackPool.empty());
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackPool.push(i);
      stackPool.push(i);
      stackPool.push(i);
      stackPool.push(i);
    }
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackPool.pop();
      stackPool.pop();
      stackPool.pop();
      stackPool.pop();
    }
  }
  std::cout << "MemoryPool Allocator Time: ";
  std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";


  std::cout << "Here is a secret: the best way of implementing a stack"
            " is a dynamic array.\n";

  /* Compare MemoryPool to std::vector */
  std::vector<int> stackVector;
  start = clock();
  for (int j = 0; j < REPS; j++)
  {
    assert(stackVector.empty());
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackVector.push_back(i);
      stackVector.push_back(i);
      stackVector.push_back(i);
      stackVector.push_back(i);
    }
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackVector.pop_back();
      stackVector.pop_back();
      stackVector.pop_back();
      stackVector.pop_back();
    }
  }
  std::cout << "Vector Time: ";
  std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

  std::cout << "The vector implementation will probably be faster.\n\n";
  std::cout << "MemoryPool still has a lot of uses though. Any type of tree"
            " and when you have multiple linked lists are some examples (they"
            " can all share the same memory pool).\n";

  return 0;
}
