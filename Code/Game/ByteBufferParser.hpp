#pragma once
#include"Game/GameCommon.hpp"

#include <vector>

enum class endian
{
	ENDIAN_LITTLE = 0,
	ENDIAN_BIG = 1,
	ENDIAN_NATIVE = ENDIAN_LITTLE
};

class ByteBufferParser
{
	ByteBufferParser(std::vector<uchar>& buffer);
	~ByteBufferParser();

	
	// functions left
	//		Boolean types
	//			bool
	//		integer
	//			short
	//			unsigned short
	//			long
	//			unsigned long
	//			long long
	//			unsigned long long int
	//			__int8
	//			__int16
	//			__int32
	//			__int64
	//		Character 
	//			signed char
	//			unsigned char
	//			wchar_t
	//			char8_t
	//			char16_t
	//			char32_t
	//		floating point
	//			double
	//			long double

	uchar	ParseByte();
	char	ParseChar();	
	int		ParseInt();
	uint		ParseUnsignedInt();
	float	ParseFloat();
	char*	ParseByteArray(uint size);
	
	void SetHead(uint idx);
	uint GetBufferSize() const;
	
	std::vector<uchar>&		m_buffer;
	uint					m_head = 0;

};
