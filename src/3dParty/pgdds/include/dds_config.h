/**
 * @file dds_config.h
 * @brief DDS Configuration Interface
 * 
 * This header defines configuration structures for DDS initialization
 * allowing programmatic configuration without XML files.
 */

#ifndef DDS_CONFIG_H
#define DDS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/* DLL export/import macros */
#ifdef _WIN32
    #ifdef DDS_CONFIG_EXPORTS
        #define DDS_CONFIG_API __declspec(dllexport)
    #else
        #define DDS_CONFIG_API __declspec(dllimport)
    #endif
#else
    #define DDS_CONFIG_API
#endif

/**
 * @brief Network interface configuration
 * 
 * Controls which network interfaces DDS uses for communication.
 */
typedef struct NetworkInterfaceConfig {
    const char* interface_name;      /**< Network interface name (e.g., "eth0", "en0") */
    const char* allow_interface;     /**< Comma-separated list of allowed interfaces (e.g., "eth0,eth1") */
    const char* deny_interface;      /**< Comma-separated list of denied interfaces */
    const char* bind_address;        /**< Bind to specific IP address (e.g., "192.168.1.100") */
    const char* multicast_address;   /**< Multicast address (e.g., "239.255.0.1") */
    int enable_multicast;            /**< Enable multicast: 1=enable, 0=disable */
} NetworkInterfaceConfig;

/**
 * @brief Discovery configuration
 * 
 * Controls how DDS participants discover each other.
 */
typedef struct DiscoveryConfig {
    int discovery_protocol;          /**< Discovery protocol: 0=SIMPLE, 1=CLIENT, 2=SERVER, 3=BACKUP */
    const char* initial_peers;       /**< Comma-separated list of initial peer addresses (e.g., "192.168.1.10,192.168.1.11") */
    const char* server_prefix;       /**< Server prefix for SERVER mode */
    int lease_duration_sec;          /**< Lease duration in seconds (0 = use default) */
    int announcement_period_sec;     /**< Announcement period in seconds (0 = use default) */
    /* Dynamic Service Discovery Extensions */
    int dynamic_enabled;             /**< Enable dynamic discovery via external service: 1=enable, 0=disable */
    const char* discovery_service_url; /**< Base URL of discovery service (e.g., "http://192.168.1.50:8080") */
    int heartbeat_interval_sec;      /**< Heartbeat interval seconds to refresh registration (0 = auto) */
    int ttl_sec;                     /**< TTL seconds for registration (0 = use lease_duration_sec or default) */
} DiscoveryConfig;

/**
 * @brief Transport layer configuration
 * 
 * Controls which transport protocols DDS uses.
 */
typedef struct TransportConfig {
    int use_udpv4;                   /**< Use UDPv4: 1=enable, 0=disable */
    int use_udpv6;                   /**< Use UDPv6: 1=enable, 0=disable */
    int use_tcpv4;                   /**< Use TCPv4: 1=enable, 0=disable (FastDDS only) */
    int use_shm;                     /**< Use shared memory: 1=enable, 0=disable (for same-host communication) */
    uint16_t udp_port;               /**< UDP port (0 = auto-assign) */
    uint16_t tcp_port;               /**< TCP port (0 = auto-assign, FastDDS only) */
    int send_socket_buffer_size;     /**< Send buffer size in bytes (0 = use default) */
    int recv_socket_buffer_size;     /**< Receive buffer size in bytes (0 = use default) */
    int max_message_size;            /**< Maximum message size in bytes (0 = use default) */
    int ttl;                         /**< Time To Live (TTL) for multicast (0 = use default) */
} TransportConfig;

/**
 * @brief Resource limits configuration
 * 
 * Controls resource allocation limits for DDS entities.
 */
typedef struct ResourceLimitsConfig {
    int max_participants;            /**< Maximum number of participants (0 = unlimited) */
    int max_readers_per_participant; /**< Maximum readers per participant (0 = unlimited) */
    int max_writers_per_participant; /**< Maximum writers per participant (0 = unlimited) */
    int max_samples;                 /**< Maximum number of samples (0 = unlimited) */
    int max_instances;               /**< Maximum number of instances (0 = unlimited) */
    int max_samples_per_instance;    /**< Maximum samples per instance (0 = unlimited) */
} ResourceLimitsConfig;

/**
 * @brief Logging configuration
 * 
 * Controls DDS logging behavior.
 */
typedef struct DDSLogConfig {
    int log_level;                   /**< Log level: 0=ERROR, 1=WARNING, 2=INFO, 3=DEBUG */
    const char* log_file;            /**< Log file path (NULL = stdout) */
    int enable_timestamp;            /**< Enable timestamps: 1=enable, 0=disable */
} DDSLogConfig;

/**
 * @brief Complete DDS configuration
 * 
 * Main configuration structure combining all DDS settings.
 * All sub-configuration pointers are optional (can be NULL for defaults).
 */
typedef struct DDSConfig {
    int domain_id;                         /**< Domain ID */
    const char* participant_name;          /**< Participant name (optional) */
    NetworkInterfaceConfig* network;       /**< Network configuration (optional, NULL for default) */
    DiscoveryConfig* discovery;            /**< Discovery configuration (optional, NULL for default) */
    TransportConfig* transport;            /**< Transport configuration (optional, NULL for default) */
    ResourceLimitsConfig* resource_limits; /**< Resource limits (optional, NULL for default) */
    DDSLogConfig* log;                     /**< Logging configuration (optional, NULL for default) */
    int disable_topic_name_validation;     /**< Disable DDS topic name validation: 1=disable (default), 0=enable */
} DDSConfig;

#ifdef __cplusplus
}
#endif

#endif // DDS_CONFIG_H
