#ifndef ERROR_HPP_98A7E7BD_D219_4830_B8AD_F3FF3FF39F39
#define ERROR_HPP_98A7E7BD_D219_4830_B8AD_F3FF3FF39F39

#include <string>
#include <format>
#include <exception>

#include "defaults.hpp"

#if COMP3811_CONF_USE_STACKTRACE
#	include <stacktrace>
#endif // ~ USE_STACKTRACE


// Class used for exceptions. Unlike e.g. std::runtime_error, which only
// accepts a "fixed" string, Error provides std::print()-like formatting.
//
// Example:
//
//	throw Error( "glGetError() returned {}", glerr );
//
class Error : public std::exception
{
	public:
		template< typename... tArgs >
		explicit Error( std::format_string<tArgs...>, tArgs&&... );

	public:
		char const* what() const noexcept override;

	private:
		std::string mMsg;

#		if COMP3811_CONF_USE_STACKTRACE
		std::stacktrace mTrace = std::stacktrace::current();
		mutable std::string mBuffer;
#		endif // ~ USE_STACKTRACE
};

template< typename... tArgs > inline
Error::Error( std::format_string<tArgs...> aFmt, tArgs&&... aArgs )
	: mMsg( std::format( aFmt, std::forward<tArgs>(aArgs)... ) )
{}

#endif // ERROR_HPP_98A7E7BD_D219_4830_B8AD_F3FF3FF39F39
