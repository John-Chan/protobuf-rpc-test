// codec_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <google\protobuf\descriptor.h>


#include <proto\msg_codec.hpp>
#include <proto\echo.pb.h>

#include <stdio.h>

using namespace pb::echo::test;

#ifdef _MSC_VER

void print(const std::string& buf)
{
	printf("encoded to %d bytes\n", buf.size());
	for (size_t i = 0; i < buf.size(); ++i)
	{
		unsigned char uc = (unsigned char)buf[i];
		unsigned char sc = buf[i];
		if (i>=43){
			int k = 0;
		}
		printf("%2d:  0x%02x  %c\n",
			i,
			uc,
			isgraph(sc) ? sc : ' ');
	}
}
#else

void print(const std::string& buf)
{
	printf("encoded to %zd bytes\n", buf.size());
	for (size_t i = 0; i < buf.size(); ++i)
	{
		printf("%2zd:  0x%02x  %c\n",
			i,
			(unsigned char)buf[i],
			isgraph(buf[i]) ? buf[i] : ' ');
	}
}

#endif

void	testRequest(){
	EchoRequest req;
	req.set_message("hi there!");
	std::string transport = encode(req);
	print(transport);


	////////////////////////////////////////////////////////////////////
	int32_t be32 = 0;
	std::copy(transport.begin(), transport.begin() + sizeof be32, reinterpret_cast<char*>(&be32));
	int32_t len =endian::be32_to_local(be32);
	assert(len == transport.size() - sizeof(be32));

	// network library decodes length header and get the body of message
	std::string buf = transport.substr(sizeof(int32_t));
	assert(len == buf.size());

	EchoRequest* newReq = dynamic_cast<EchoRequest*>(decode(buf));
	assert(newReq != NULL);
	newReq->PrintDebugString();
	assert(newReq->DebugString() == req.DebugString());
	delete newReq;

	////////////////////////////////////////////////////////////////////
	buf[buf.size() - 6]++;  // oops, some data is corrupted
	EchoRequest* badReq = dynamic_cast<EchoRequest*>(decode(buf));
	assert(badReq == NULL);

}

int _tmain(int argc, _TCHAR* argv[])
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	testRequest();
	return 0;
}

