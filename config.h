/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* config.h inclusion marker */
#define CONFIG_H_WAS_INCLUDED 1

/* Enable low-performing debugging facilities? */
/* #undef ENABLE_DEBUG */

/* Check logger messages? */
/* #undef ENABLE_LOGGER_CHECKS */

/* Extended Kea version */
#define EXTENDED_VERSION "git 8ff2dac485c8762d1b4b3576d806347883937a9e"

/* Does this platform have some undefined pthreads behavior? */
/* #undef HAS_UNDEFINED_PTHREAD_BEHAVIOR */

/* Define to 1 if you have the <boost/asio.hpp> header file. */
#define HAVE_BOOST_ASIO_HPP 1

/* Define to 1 if you have the <boost/asio/ip/address.hpp> header file. */
#define HAVE_BOOST_ASIO_IP_ADDRESS_HPP 1

/* Define to 1 if you have the <boost/bind.hpp> header file. */
#define HAVE_BOOST_BIND_HPP 1

/* Define to 1 if you have the
   <boost/date_time/posix_time/posix_time_types.hpp> header file. */
#define HAVE_BOOST_DATE_TIME_POSIX_TIME_POSIX_TIME_TYPES_HPP 1

/* Define to 1 if you have the <boost/foreach.hpp> header file. */
#define HAVE_BOOST_FOREACH_HPP 1

/* Define to 1 if you have the <boost/function.hpp> header file. */
#define HAVE_BOOST_FUNCTION_HPP 1

/* Define to 1 if you have the
   <boost/interprocess/sync/interprocess_upgradable_mutex.hpp> header file. */
#define HAVE_BOOST_INTERPROCESS_SYNC_INTERPROCESS_UPGRADABLE_MUTEX_HPP 1

/* Define to 1 if you have the <boost/shared_ptr.hpp> header file. */
#define HAVE_BOOST_SHARED_PTR_HPP 1

/* Define to 1 if you have the <boost/system/error_code.hpp> header file. */
#define HAVE_BOOST_SYSTEM_ERROR_CODE_HPP 1

/* Define to 1 if you have the <botan/botan.h> header file. */
#define HAVE_BOTAN_BOTAN_H 1

/* Define to 1 if getsockopt(IPV6_USE_MIN_MTU) does not work */
/* #undef HAVE_BROKEN_GET_IPV6_USE_MIN_MTU */

/* CQL is present */
/* #undef HAVE_CQL */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <log4cplus/logger.h> header file. */
#define HAVE_LOG4CPLUS_LOGGER_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* MySQL is present */
/* #undef HAVE_MYSQL */

/* Check for optreset? */
/* #undef HAVE_OPTRESET */

/* PostgreSQL is present */
/* #undef HAVE_PGSQL */

/* Define to 1 if you have the `pselect' function. */
#define HAVE_PSELECT 1

/* Define to 1 if sockaddr has a sa_len member, and corresponding sin_len and
   sun_len */
/* #undef HAVE_SA_LEN */

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/devpoll.h> header file. */
/* #undef HAVE_SYS_DEVPOLL_H */

/* Define to 1 if you have the <sys/filio.h> header file. */
/* #undef HAVE_SYS_FILIO_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <utils/errcodes.h> header file. */
/* #undef HAVE_UTILS_ERRCODES_H */

/* Check valgrind headers */
#define HAVE_VALGRIND_HEADERS 1

/* Define to 1 if you have the <valgrind/valgrind.h> header file. */
#define HAVE_VALGRIND_VALGRIND_H 1

/* Define to 1 if the system has the type `_Bool'. */
/* #undef HAVE__BOOL */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Running on BSD? */
/* #undef OS_BSD */

/* Running on FreeBSD? */
/* #undef OS_FREEBSD */

/* Running on Linux? */
#define OS_LINUX 1

/* Running on NetBSD? */
/* #undef OS_NETBSD */

/* Running on OpenBSD? */
/* #undef OS_OPENBSD */

/* Running on OSX? */
/* #undef OS_OSX */

/* Running on Solaris? */
/* #undef OS_SOLARIS */

/* Name of package */
#define PACKAGE "kea"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "kea-dev@lists.isc.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "kea"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "kea 1.1.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "kea"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.1.0"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Was Kea statically linked? */
/* #undef USE_STATIC_LINK */

/* Version number of package */
#define VERSION "1.1.0"

/* Compile with Botan crypto */
#define WITH_BOTAN /**/

/* Compile with OpenSSL crypto */
/* #undef WITH_OPENSSL */

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
/* #undef YYTEXT_POINTER */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */
