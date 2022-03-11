#include "Firefly/AssetsLoaders/Types.hpp"

namespace Firefly
{
	bool ObjVertex::operator==(const ObjVertex& other) const
	{
		return m_Position == other.m_Position && m_Color == other.m_Color && m_TextureCoordinate == other.m_TextureCoordinate;
	}
}