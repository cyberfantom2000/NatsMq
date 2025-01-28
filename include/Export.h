
#ifndef NATSMQ_EXPORT_H
#define NATSMQ_EXPORT_H

#ifdef NATSMQ_STATIC_DEFINE
#  define NATSMQ_EXPORT
#  define NATSMQ_NO_EXPORT
#else
#  ifndef NATSMQ_EXPORT
#    ifdef NatsMq_EXPORTS
        /* We are building this library */
#      define NATSMQ_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define NATSMQ_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef NATSMQ_NO_EXPORT
#    define NATSMQ_NO_EXPORT 
#  endif
#endif

#ifndef NATSMQ_DEPRECATED
#  define NATSMQ_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef NATSMQ_DEPRECATED_EXPORT
#  define NATSMQ_DEPRECATED_EXPORT NATSMQ_EXPORT NATSMQ_DEPRECATED
#endif

#ifndef NATSMQ_DEPRECATED_NO_EXPORT
#  define NATSMQ_DEPRECATED_NO_EXPORT NATSMQ_NO_EXPORT NATSMQ_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef NATSMQ_NO_DEPRECATED
#    define NATSMQ_NO_DEPRECATED
#  endif
#endif

#endif /* NATSMQ_EXPORT_H */
