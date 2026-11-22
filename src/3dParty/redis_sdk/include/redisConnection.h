#pragma once

#include "redis_middleware_interface.h"
#include <string>

struct redisContext;
class REDIS_MIDDLEWARE_EXPORT RedisConnection
{

    friend class RedisReciver;
private:
    redisContext* context_;
    std::string host_;
    int port_;
    std::string password_;
    int db_;
public:
    RedisConnection(const char* host, int port, const char* password = nullptr, int db = 0);
    ~RedisConnection();

    redisContext* getConnection() const;

    

private:
    bool connect();
protected:
    bool disconnect();
  
};