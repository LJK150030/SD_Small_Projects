#include "Game/ByteBufferParser.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

ByteBufferParser::ByteBufferParser(std::vector<uchar>& buffer) : m_buffer(buffer)
{
	//TODO this is file agnostic, buffer parser only cares about parsing an STD vector given from construction 
	
	// 	m_buffer.clear();
	// 	m_buffer.reserve(size);
	// 
	//  FILE* file;
	// 	errno_t err;
	// 	const char* dir;
	// 	uint size = 65'535;
	// 	
	// 	err = fopen_s(&file, dir, "rb");
	// 
	// 	if(err != 0)
	// 	{
	// 		ERROR_AND_DIE("cannot open file '%s': %s\n", dir, strerror(err));
	// 	}
	// 		
	// 	fread(&m_buffer[0], sizeof(uchar), size, file);
	// 	fclose(file);
	
}


ByteBufferParser::~ByteBufferParser() = default;


uchar ByteBufferParser::ParseByte()
{
	const uint byte_size = sizeof(uchar);
	const uint new_head_idx = m_head + byte_size;
	const uint buffer_size = GetBufferSize();
	ASSERT_OR_DIE(buffer_size > new_head_idx, "Attempting to read outside buffer");

	uchar result = m_buffer[m_head];
	m_head = new_head_idx;

	return result;
}


char ByteBufferParser::ParseChar()
{
	const uint byte_size = sizeof(char);
	const uint new_head_idx = m_head + byte_size;
	const uint buffer_size = GetBufferSize();
	ASSERT_OR_DIE(buffer_size > new_head_idx, "Attempting to read outside buffer");

	char result = static_cast<char>(m_buffer[m_head]);
	m_head = new_head_idx;

	return result;
}


int ByteBufferParser::ParseInt()
{
	return 0;
}


uint ByteBufferParser::ParseUnsignedInt()
{
	return 0;
}


float ByteBufferParser::ParseFloat()
{
	return 0.0f;
}


char* ByteBufferParser::ParseByteArray(uint size)
{
	return nullptr;
}


void ByteBufferParser::SetHead(uint idx)
{
}


uint ByteBufferParser::GetBufferSize() const
{
	return static_cast<uint>(m_buffer.size());
}

