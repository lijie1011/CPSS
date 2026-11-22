#pragma once

#include "redis_middleware_interface.h"
#include "redisConnection.h"
#include <string>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>
#include "redis_protocl.h"
#include "protocol.pb.h"

struct  redisReply;
class REDIS_MIDDLEWARE_EXPORT RedisReciver
{
private:
    std::shared_ptr<RedisConnection> connection_;
    bool is_running_;
    std::string channel_;
    std::thread thread_;
    std::unordered_map<std::string, std::function<void(const std::string& data)>> callback_map_;
public:
    RedisReciver(const std::shared_ptr<RedisConnection>& connection, const char* channel);
    ~RedisReciver();
    void start();
    void stop();
    
    // 注册消息类型处理器（需要手动指定类型名）
    template<class T>
    bool registerMsgType(const std::string& type, std::function<void(T*)> callback) {
        callback_map_.emplace(type, [callback](const std::string& data) {
            T t;
            t.ParseFromString(data);
            callback(&t);
        });
        return true;
    }

 private:
    void handleMessage(redisReply* reply);
};

// 方案 1：BEGIN...END 宏（推荐，支持多行 lambda）
#define REGISTER_MSG_TYPE_BEGIN(receiver, MessageType) \
    (receiver)->registerMsgType<MessageType>(#MessageType,

#define REGISTER_MSG_TYPE_END );
