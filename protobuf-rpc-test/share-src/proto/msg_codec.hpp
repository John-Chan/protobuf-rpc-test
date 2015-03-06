#ifndef PROTO_MESSAGE_CODEC_HPP__
#define PROTO_MESSAGE_CODEC_HPP__

/*
message structure
struct ProtobufTransportFormat __attribute__ ((__packed__))
{
int32_t  len;
int32_t  nameLen;
char     typeName[nameLen];
char     protobufData[len-nameLen-8];
int32_t  checkSum; // adler32 of nameLen, typeName and protobufData
};

*/

#include <string>
#include <boost/cstdint.hpp>
#include <google/protobuf/message.h>

#include "endianess.hpp"
#include "zlib_adler32.hpp"

static const int kHeaderLen = sizeof(boost::int32_t);

///
/// Encode protobuf Message to transport format defined above
/// returns a std::string.
///
/// returns a empty string if message.AppendToString() fails.
///
inline std::string encode(const google::protobuf::Message& message)
{
	std::string result;

	result.resize(kHeaderLen);

	const std::string& typeName = message.GetTypeName();
	int32_t nameLen = static_cast<int32_t>(typeName.size() + 1);
	int32_t be32 = endian::be32_to_local(nameLen);
	result.append(reinterpret_cast<char*>(&be32), sizeof be32);
	result.append(typeName.c_str(), nameLen);
	bool succeed = message.AppendToString(&result);

	if (succeed)
	{
		const char* begin = result.c_str() + kHeaderLen;
		int32_t checkSum =adler32(1, reinterpret_cast<const unsigned char*>(begin), result.size() - kHeaderLen);
		int32_t be32 = endian::be32_to_local(checkSum);
		result.append(reinterpret_cast<char*>(&be32), sizeof be32);

		int32_t len = endian::be32_to_local(result.size() - kHeaderLen);
		std::copy(reinterpret_cast<char*>(&len),
			reinterpret_cast<char*>(&len) + sizeof len,
			result.begin());
	}
	else
	{
		result.clear();
	}

	return result;
}

inline google::protobuf::Message* createMessage(const std::string& type_name)
{
	google::protobuf::Message* message = NULL;
	const google::protobuf::Descriptor* descriptor =
		google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
	if (descriptor)
	{
		const google::protobuf::Message* prototype =
			google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype)
		{
			message = prototype->New();
		}
	}
	return message;
}

inline int32_t asInt32(const char* buf)
{
	int32_t be32 = 0;
	::memcpy(&be32, buf, sizeof(be32));
	return endian::local_to_be32(be32);
}

///
/// Decode protobuf Message from transport format defined above.
/// returns a Message*
///
/// returns NULL if fails.
///
inline google::protobuf::Message* decode(const std::string& buf)
{
	google::protobuf::Message* result = NULL;

	int32_t len = static_cast<int32_t>(buf.size());
	if (len >= 10)
	{
		int32_t expectedCheckSum = asInt32(buf.c_str() + buf.size() - kHeaderLen);
		const char* begin = buf.c_str();
		int32_t checkSum = adler32(1, reinterpret_cast<const unsigned char*>(begin), len - kHeaderLen);
		if (checkSum == expectedCheckSum)
		{
			int32_t nameLen = asInt32(buf.c_str());
			if (nameLen >= 2 && nameLen <= len - 2 * kHeaderLen)
			{
				std::string typeName(buf.begin() + kHeaderLen, buf.begin() + kHeaderLen + nameLen - 1);
				google::protobuf::Message* message = createMessage(typeName);
				if (message)
				{
					const char* data = buf.c_str() + kHeaderLen + nameLen;
					int32_t dataLen = len - nameLen - 2 * kHeaderLen;
					if (message->ParseFromArray(data, dataLen))
					{
						result = message;
					}
					else
					{
						// parse error
						delete message;
					}
				}
				else
				{
					// unknown message type
				}
			}
			else
			{
				// invalid name len
			}
		}
		else
		{
			// check sum error
		}
	}

	return result;
}

#endif	// !PROTO_MESSAGE_CODEC_HPP__