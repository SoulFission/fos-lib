FOS ("Fission Of Soul") general-purpose C library. Requires at least C11-compliant compiler (GCC/Clang/Pelles C). 
MSVS doesn't support _Thread_local, but a workaround is included.

The modules use leaf-based design, so they are not dependent on each other.

Currently implemented: 
- FOS_Rng (a pseudo-random numbers generator)
- FOS_HashMap (a key/value storage)
- FOS_Vec (a dynamic array)
- FOS_String (strings with owned data, non-owning slices; no Unicode support yet)

Documentation and examples will be added later.
