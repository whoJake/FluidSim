#pragma once

#define MAKE_MAGIC_NUMBER(a, b, c, d) \
    ((d << 24) +\
     (c << 16) +\
     (b << 8) +\
     (a))

#define MAKE_MAGIC_NUMBER_8(a, b, c, d, e, f, g, h) \
    ((g << 56) +\
     (f << 48) +\
     (e << 32) +\
     (d << 24) +\
     (c << 16) +\
     (b << 8) +\
     (a))