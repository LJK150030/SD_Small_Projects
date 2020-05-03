#pragma once
#include "Game/GameCommon.hpp"

#include <vector>


class ByteBufferWriter
{
	ByteBufferWriter();
	~ByteBufferWriter();

	
	std::vector<uchar> m_buffer;
};
