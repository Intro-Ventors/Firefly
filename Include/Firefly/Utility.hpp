#pragma once

#include "Imports.hpp"
#include "BackendError.hpp"

#include <functional>

namespace Firefly
{
	namespace Utility
	{
		/**
		 * Log level enum.
		 * This specifies the log level.
		 */
		enum class LogLevel : uint8_t
		{
			Information,
			Warning,
			Error,
			Fatal
		};

		/**
		 * Validate the incoming result.
		 * This will throw an exception depending on the result provided. If the result is VK_SUCCESS, it will not do anything.
		 *
		 * @param result The result to be validated.
		 * @param string The output message.
		 * @throws std::runtime_error depending on the result.
		 */
		void ValidateResult(const VkResult result, const std::string& string);

		/**
		 * Validate the incoming result.
		 * This will throw an exception depending on the result provided. If the result is VK_SUCCESS, it will not do anything.
		 *
		 * @param result The result to be validated.
		 * @param string The output message.
		 * @param file The file name which threw the error.
		 * @param line The line number which called this function.
		 * @throws std::runtime_error depending on the result.
		 */
		void ValidateResult(const VkResult result, const std::string& string, const std::string_view& file, const uint64_t line);

		/**
		 * Set the logger method.
		 * This method will be called when the log function is called.
		 *
		 * @parm function The log function.
		 */
		void SetLoggerMethod(const std::function<void(const LogLevel level, const std::string_view&)>& function);

		/**
		 * Log a message.
		 *
		 * @param level The message level.
		 * @param message The output message.
		 */
		void Log(const LogLevel level, const std::string_view& message);
	}
}

#define FIREFLY_DEFAULT_COPY(name)					name(const name&) = default;	name& operator=(const name&) = default
#define FIREFLY_DEFAULT_MOVE(name)					name(name&&) = default;			name& operator=(name&&) = default

#define FIREFLY_NO_COPY(name)						name(const name&) = delete;		name& operator=(const name&) = delete
#define FIREFLY_NO_MOVE(name)						name(name&&) = delete;			name& operator=(name&&) = delete

#define FIREFLY_VALIDATE(expression, message)		::Firefly::Utility::ValidateResult(expression, message, __FILE__, __LINE__)