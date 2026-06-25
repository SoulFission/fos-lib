FOS ("Fission Of Soul") is a general-purpose C library. Requires at least C11-compliant compiler (GCC/Clang/Pelles C). 
MSVS doesn't support `_Thread_local`, but a workaround is included.

The following modules use leaf-based design, so they are not dependent on each other: 
- FOS_Rng (a pseudo-random numbers generator)
- FOS_HashMap (a key/value storage)
- FOS_Vec (a dynamic array)
- FOS_String (strings with owned data, non-owning slices; no Unicode support yet)
- FOS_Bignum (arbitrary-precision integers and arithmetic)
- FOS_List (a generic singly-linked list)

But every module listed above depends on a custom configurable allocator:
- FOS_Memory

---

This module depends on FOS_HashMap:
- FOS_Set (a container of unordered unique values)

These modules depend on FOS_List:
- FOS_Stack (a generic LIFO container)
- FOS_Queue (a generic FIFO container)

This module depends on FOS_Vec and FOS_Memory:
- FOS_Heap (a priority queue using a binary heap stored in a dynamic array)

Documentation and examples will be added later.
