#pragma once


#include "redis_middleware_interface.h"
#include "redisConnection.h"
#include <memory>
#include "redis_protocl.h"
#include "protocol.pb.h"
#include <thread>
#include <mutex>
#include <deque>

template<class T>
struct PackData {
    static std::string pack(std::string type, const T* v) {
        HTProto::Envelope env;
        env.set_type(type);
        env.mutable_payload()->PackFrom(*v);
        std::string buf;
        env.SerializeToString(&buf);
        return buf;
    }
};


class REDIS_MIDDLEWARE_EXPORT RedisPublisher
{
private:
    std::shared_ptr<RedisConnection> connection_;
public:
    RedisPublisher(const std::shared_ptr<RedisConnection>& connection);
    ~RedisPublisher();

    template<class T>
    bool publishMesssage(const char* channel, const std::string& type, const T* data) {
        HTProto::Envelope env;
        env.set_type(type);
        
        std::string payload_data;
        data->SerializeToString(&payload_data);
        env.set_payload(payload_data);
        
        std::string buf;
        env.SerializeToString(&buf);
        
        return publishMesssage(channel, buf.c_str(), buf.length());
    }

    void start();

    void stop();

private:
    bool  publishMesssage(const char* channel, const char* data, int len);

private:
    std::deque<std::pair<std::string, std::string>> messagePool;
    std::thread SendThread;
    std::mutex mut;
    bool bStop = true;

};

// ������Ϣ���Զ�ʹ����������Ϊ�ַ�����
#define PUBLISH_MSG(publisher, channel, MessageType, msg_ptr) \
    (publisher)->publishMesssage(channel, #MessageType, msg_ptr)

