#ifndef DEFAULTS_HPP_15153D57_59D5_4A86_A2F7_77B24A50AA8A
#define DEFAULTS_HPP_15153D57_59D5_4A86_A2F7_77B24A50AA8A

#include <version>

/* Compile time config: Use <stacktrace>?
 *
 * Enabled by default unless NDEBUG is specified. If your compiler doesn't
 * support it, you can disable it by defining COMP3811_CONF_USE_STACKTRACE=0.
 */
#if !defined(COMP3811_CONF_USE_STACKTRACE)
#	if defined(NDEBUG)
#		define COMP3811_CONF_USE_STACKTRACE 0
#	else
#		if defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011L
#			define COMP3811_CONF_USE_STACKTRACE 1
#		else /* unsupported */
#			define COMP3811_CONF_USE_STACKTRACE 0
#		endif
#	endif
#endif // ~ COMP3811_CONF_USE_STACKTRACE

#endif // DEFAULTS_HPP_15153D57_59D5_4A86_A2F7_77B24A50AA8A
