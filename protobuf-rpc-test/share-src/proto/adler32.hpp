#ifndef ADLER32_CHECKSUM_HPP__
#define ADLER32_CHECKSUM_HPP__

#include <boost/cstdint.hpp>

static const int MOD_ADLER = 65521;

/* 
where data is the location of the data in physical memory and
len is the length of the data in bytes 
*/
inline boost::uint32_t calc_adler32(const unsigned char *data, size_t len) 
{
	boost::uint32_t a = 1, b = 0;
	size_t index;

	/* Process each byte of the data in order */
	for (index = 0; index < len; ++index)
	{
		a = (a + data[index]) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;
	}

	return (b << 16) | a;
}
#endif// !ADLER32_CHECKSUM_HPP__