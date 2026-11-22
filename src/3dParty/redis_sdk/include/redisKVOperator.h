#pragma once
#include "redis_middleware_interface.h"
#include <memory>
#include <string>
#include <map>

class RedisConnection;
class REDIS_MIDDLEWARE_EXPORT RedisKVOperator
{
public:
	RedisKVOperator(const std::shared_ptr<RedisConnection>& connection);
	~RedisKVOperator();

	template<class T>
	bool storeMsg(const std::string& key, const T* data) {
		std::string payload_data;
		if (!data->SerializeToString(&payload_data)) {
			printf("SerializeToString failed\n");
			return false;
		}
		return storgeMsg(key.c_str(), payload_data.data(), payload_data.size());
	}

	template<class T>
	T readMsg(const std::string& key) {
		std::string buf = readMsgImpl(key.c_str());
		T ret;
		ret.ParseFromArray(buf.c_str(), buf.length());
		return ret;
	}

	std::string readMessge(const char* key) {
		std::string buf = readMsgImpl(key);
		return buf;
	}

	void readHMessge(const char* hash, std::map<std::string, std::string>& ret) {
		hReadMsgImpl(hash, ret);
	}

	std::string readHMessge(const char* hash, const char* key) {
		return hReadMsgImpl(hash, key);
	}


private:
	bool storgeMsg(const char* key, const char* data, size_t  data_size);

	std::string readMsgImpl(const char* key);

	void hReadMsgImpl(const char* hash, std::map<std::string, std::string>& ret);

	std::string hReadMsgImpl(const char* hash, const char* key);

private:
	std::shared_ptr<RedisConnection> _connection;
};

#define STORE_MSG(op, MessageType, msg_ptr) \
    (op)->storeMsg(#MessageType, msg_ptr)

#define GET_MSG(op, MessageType) \
    (op)->readMsg<MessageType>(#MessageType)