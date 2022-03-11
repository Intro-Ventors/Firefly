#include "Firefly/Utility.hpp"

namespace Firefly
{
	namespace Utility
	{
		void ValidateResult(const VkResult result, const std::string_view& string)
		{
			if (result != VkResult::VK_SUCCESS)
				throw BackendError(string);
		}
	}
}
