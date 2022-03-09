#pragma once

#include <stdexcept>
#include <string>

namespace Firefly
{
	/**
	 * Backend error exception class.
	 */
	class BackendError final : public std::runtime_error
	{
	public:
		/**
		 * Explicit constructor.
		 *
		 * @param message The exception message.
		 */
		explicit BackendError(const std::string_view& message) : std::runtime_error(message.data()) {}
	};
}