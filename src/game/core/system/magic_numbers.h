#pragma once

#define MAKE_MAGIC_NUMBER(a, b, c, d) \
    ((a << 24) +\
     (b << 16) +\
     (c << 8) +\
     (d))

#define MAKE_MAGIC_NUMBER_8(a, b, c, d, e, f, g, h) \
    ((a << 56) +\
     (b << 48) +\
     (c << 32) +\
     (d << 24) +\
     (e << 16) +\
     (f << 8) +\
     (g))