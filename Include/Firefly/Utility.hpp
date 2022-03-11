#pragma once

#include "Imports.hpp"
#include "BackendError.hpp"

namespace Firefly
{
	namespace Utility
	{
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
		 * Log a message to the console.
		 * 
		 * @param result The result to be validated.
		 * @param string The output message.
		 * @param file The file name which threw the error.
		 * @param line The line number which called this function.
		 */
		void Logger(const VkResult result, const std::string& string, const std::string_view& file, const uint64_t line);
	}
}

#define FIREFLY_DEFAULT_COPY(name)					name(const name&) = default;	name& operator=(const name&) = default
#define FIREFLY_DEFAULT_MOVE(name)					name(name&&) = default;			name& operator=(name&&) = default

#define FIREFLY_NO_COPY(name)						name(const name&) = delete;		name& operator=(const name&) = delete
#define FIREFLY_NO_MOVE(name)						name(name&&) = delete;			name& operator=(name&&) = delete

#ifdef _DEBUG
#define FIREFLY_VALIDATE(expression, message)		::Firefly::Utility::ValidateResult(expression, message, __FILE__, __LINE__)

#else
#define FIREFLY_VALIDATE(expression, message)		::Firefly::Utility::Logger(expression, message, __FILE__, __LINE__)

#endif