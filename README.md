# dstr
An stb-style(single-file, header-only) dynamic string library in c.

> [!WARNING]
> This library was created for my own learning purpose. This library is WIP and untested in production.
> If you choose to use it, do it at your own risk.

## Usage
- Copy [dstr.h](./dstr.h) into your project.
- Include the library like this:
```c
#define DSTR_IMPLEMENTATION
#include "dstr.h"
```
- Note: define `DSTR_IMPLEMENTATION` only once in your project. For more information, see [stb_howto.txt](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt)
- This library uses `realloc` and `free` of `stdlib` to allocate and free memory. If you want to use your own custom allocator and free, you can do that by defining `DSTR_ALLOC` and `DSTR_FREE`. Note that you must define both or neither.

## License
[MIT License](./LICENSE)
