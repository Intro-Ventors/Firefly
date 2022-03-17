#pragma once

#include "Imports.hpp"
#include "BackendError.hpp"

#define FIREFLY_DEFAULT_COPY(name)					name(const name&) = default;	name& operator=(const name&) = default
#define FIREFLY_DEFAULT_MOVE(name)					name(name&&) = default;			name& operator=(name&&) = default

#define FIREFLY_NO_COPY(name)						name(const name&) = delete;		name& operator=(const name&) = delete
#define FIREFLY_NO_MOVE(name)						name(name&&) = delete;			name& operator=(name&&) = delete

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
		 * Logger class.
		 * This class is a singleton, and is used to log messages using a logger function.
		 */
		class Logger
		{
			Logger() = default;
			FIREFLY_NO_COPY(Logger);
			FIREFLY_NO_MOVE(Logger);

			std::function<void(const Firefly::Utility::LogLevel, const std::string_view&)> m_Function;
			using Function = void(*)(const Firefly::Utility::LogLevel, const std::string_view&);
			Function m_Function = nullptr;

			/**
			 * Get the logger instance.
			 *
			 * @return The logger instance reference.
			 */
			static Logger& getInstance();
		public:
			/**
			 * Set the logger method to use.
			 *
			 * @param function The logger function.
			 */
			static void setLoggerMethod(const Function& function);

			/**
			 * Log a message.
			 *
			 * @param level The log level.
			 * @param message The message to be logged.
			 */
			static void log(const Firefly::Utility::LogLevel level, const std::string_view& message);
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
	}
}

#define FIREFLY_VALIDATE(expression, message)		::Firefly::Utility::ValidateResult(expression, message, __FILE__, __LINE__)