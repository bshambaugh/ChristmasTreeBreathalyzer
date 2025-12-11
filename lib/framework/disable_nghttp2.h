#pragma once

// If nghttp2 tries to define `HTTP_*` enums, disable its macro
#ifdef HTTP_METHOD_MAP
#undef HTTP_METHOD_MAP
#endif

#define HTTP_METHOD_MAP(XX)   /* nothing */
