/*
 * simple override for _long_to_time()
 */
#ifndef _TIMECONV_H_
#define _TIMECONV_H_
static __inline time_t
_long_to_time(long tlong)
{
    if (sizeof(long) == sizeof(__int32_t))
        return((time_t)(__int32_t)(tlong));
    return((time_t)tlong);
}

#ifdef __linux__

/*
 * some linux headers have variables called __unused, whereas the name
 * is an alias for the gcc attribute on FreeBSD.
 * We have to define __unused appropriately, but this cannot be
 * global because it would clash with the linux headers.
 *
 * __unused is defined here because there is not a better place
 * and this file is included by ipfw2.c where the offending linux
 * headers are not included.
 */
#define __unused       __attribute__ ((__unused__))
#endif

#endif /* _TIMECONV_H_ */
