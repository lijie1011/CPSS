/**
 * @file dds_config_builder.h
 * @brief DDS Configuration Builder Interface
 * 
 * This header provides helper functions for building DDS configurations
 * using a Builder pattern for easier configuration management.
 */

#ifndef DDS_CONFIG_BUILDER_H
#define DDS_CONFIG_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dds_config.h"

/* DLL export/import macros */
#ifdef _WIN32
    #ifdef DDS_CONFIG_EXPORTS
        #define DDS_CONFIG_BUILDER_API __declspec(dllexport)
    #else
        #define DDS_CONFIG_BUILDER_API __declspec(dllimport)
    #endif
#else
    #define DDS_CONFIG_BUILDER_API
#endif

/**
 * @brief Create a default DDS configuration
 * 
 * @param domain_id Domain ID to use
 * @return Pointer to newly allocated DDSConfig (must be freed with dds_config_destroy)
 */
DDS_CONFIG_BUILDER_API DDSConfig* dds_config_create_default(int domain_id);

/**
 * @brief Destroy and free a DDS configuration
 * 
 * @param config Configuration to destroy
 */
DDS_CONFIG_BUILDER_API void dds_config_destroy(DDSConfig* config);

/* Network Configuration Functions */

/**
 * @brief Set network interface by name
 * 
 * @param config Configuration to modify
 * @param interface_name Interface name (e.g., "eth0", "en0")
 */
DDS_CONFIG_BUILDER_API void dds_config_set_network_interface(DDSConfig* config, const char* interface_name);

/**
 * @brief Set bind address
 * 
 * @param config Configuration to modify
 * @param ip_address IP address to bind to (e.g., "192.168.1.100")
 */
DDS_CONFIG_BUILDER_API void dds_config_set_bind_address(DDSConfig* config, const char* ip_address);

/**
 * @brief Set allowed network interfaces
 * 
 * @param config Configuration to modify
 * @param interfaces Comma-separated list of interfaces (e.g., "eth0,eth1")
 */
DDS_CONFIG_BUILDER_API void dds_config_set_allowed_interfaces(DDSConfig* config, const char* interfaces);

/**
 * @brief Set denied network interfaces
 * 
 * @param config Configuration to modify
 * @param interfaces Comma-separated list of interfaces to deny
 */
DDS_CONFIG_BUILDER_API void dds_config_set_denied_interfaces(DDSConfig* config, const char* interfaces);

/**
 * @brief Enable or disable multicast
 * 
 * @param config Configuration to modify
 * @param enable 1 to enable, 0 to disable
 */
DDS_CONFIG_BUILDER_API void dds_config_set_multicast(DDSConfig* config, int enable);

/**
 * @brief Set multicast address
 * 
 * @param config Configuration to modify
 * @param address Multicast address (e.g., "239.255.0.1")
 */
DDS_CONFIG_BUILDER_API void dds_config_set_multicast_address(DDSConfig* config, const char* address);

/**
 * @brief Set multicast TTL (Time To Live)
 * 
 * Controls how many network hops (routers) multicast packets can traverse.
 * Critical for multi-VLAN environments where DDS nodes need to discover each other
 * across different virtual networks.
 * 
 * @param config Configuration to modify
 * @param ttl TTL value (1-255):
 *            - 1: Local subnet only (default, prevents cross-VLAN discovery)
 *            - 2-10: Limited network scope (recommended for enterprise LANs with multiple VLANs)
 *            - 32-64: Wide area networks
 *            - 255: Unrestricted (use with caution)
 * 
 * @note For multi-VLAN deployments, set TTL to at least the number of router hops
 *       between VLANs plus 1. Example: 2 VLANs with 1 router between them → ttl=2
 * @note Default value is 1 (local subnet only)
 */
DDS_CONFIG_BUILDER_API void dds_config_set_multicast_ttl(DDSConfig* config, int ttl);

/* Discovery Configuration Functions */

/**
 * @brief Set initial peers for discovery
 * 
 * @param config Configuration to modify
 * @param peers Comma-separated list of peer addresses (e.g., "192.168.1.10,192.168.1.20")
 */
DDS_CONFIG_BUILDER_API void dds_config_set_initial_peers(DDSConfig* config, const char* peers);

/**
 * @brief Set discovery protocol
 * 
 * @param config Configuration to modify
 * @param protocol Discovery protocol: 0=SIMPLE, 1=CLIENT, 2=SERVER, 3=BACKUP
 */
DDS_CONFIG_BUILDER_API void dds_config_set_discovery_protocol(DDSConfig* config, int protocol);

/**
 * @brief Set lease duration for discovery
 * 
 * @param config Configuration to modify
 * @param duration_sec Lease duration in seconds
 */
DDS_CONFIG_BUILDER_API void dds_config_set_lease_duration(DDSConfig* config, int duration_sec);

/* Dynamic Discovery Service Configuration Functions */
/**
 * @brief Enable or disable dynamic discovery using external service
 * @param config Configuration to modify
 * @param enable 1 to enable, 0 to disable
 */
DDS_CONFIG_BUILDER_API void dds_config_enable_dynamic_discovery(DDSConfig* config, int enable);

/**
 * @brief Set discovery service base URL
 * @param config Configuration to modify
 * @param url Base URL (e.g., "http://host:port")
 */
DDS_CONFIG_BUILDER_API void dds_config_set_discovery_service_url(DDSConfig* config, const char* url);

/**
 * @brief Set dynamic discovery heartbeat interval in seconds
 * @param config Configuration to modify
 * @param heartbeat_sec Interval seconds
 */
DDS_CONFIG_BUILDER_API void dds_config_set_discovery_heartbeat(DDSConfig* config, int heartbeat_sec);

/**
 * @brief Set dynamic discovery TTL seconds (registration validity)
 * @param config Configuration to modify
 * @param ttl_sec TTL seconds
 */
DDS_CONFIG_BUILDER_API void dds_config_set_discovery_ttl(DDSConfig* config, int ttl_sec);

/* Transport Configuration Functions */

/**
 * @brief Set UDP port
 * 
 * @param config Configuration to modify
 * @param port UDP port (0 for auto-assign)
 */
DDS_CONFIG_BUILDER_API void dds_config_set_udp_port(DDSConfig* config, uint16_t port);

/**
 * @brief Set TCP port (FastDDS only)
 * 
 * @param config Configuration to modify
 * @param port TCP port (0 for auto-assign)
 */
DDS_CONFIG_BUILDER_API void dds_config_set_tcp_port(DDSConfig* config, uint16_t port);

/**
 * @brief Enable or disable UDPv4 transport
 * 
 * @param config Configuration to modify
 * @param enable 1 to enable, 0 to disable
 */
DDS_CONFIG_BUILDER_API void dds_config_enable_udpv4(DDSConfig* config, int enable);

/**
 * @brief Enable or disable UDPv6 transport
 * 
 * @param config Configuration to modify
 * @param enable 1 to enable, 0 to disable
 */
DDS_CONFIG_BUILDER_API void dds_config_enable_udpv6(DDSConfig* config, int enable);

/**
 * @brief Enable or disable shared memory transport
 * 
 * @param config Configuration to modify
 * @param enable 1 to enable, 0 to disable
 */
DDS_CONFIG_BUILDER_API void dds_config_enable_shared_memory(DDSConfig* config, int enable);

/**
 * @brief Enable or disable TCPv4 transport (FastDDS only)
 * 
 * @param config Configuration to modify
 * @param enable 1 to enable, 0 to disable
 */
DDS_CONFIG_BUILDER_API void dds_config_enable_tcpv4(DDSConfig* config, int enable);

/**
 * @brief Set socket buffer sizes
 * 
 * @param config Configuration to modify
 * @param send_size Send buffer size in bytes
 * @param recv_size Receive buffer size in bytes
 */
DDS_CONFIG_BUILDER_API void dds_config_set_socket_buffer_size(DDSConfig* config, int send_size, int recv_size);

/**
 * @brief Set maximum message size
 * 
 * @param config Configuration to modify
 * @param size Maximum message size in bytes
 */
DDS_CONFIG_BUILDER_API void dds_config_set_max_message_size(DDSConfig* config, int size);

/* Resource Limits Functions */

/**
 * @brief Set maximum number of participants
 * 
 * @param config Configuration to modify
 * @param max_participants Maximum participants (0 = unlimited)
 */
DDS_CONFIG_BUILDER_API void dds_config_set_max_participants(DDSConfig* config, int max_participants);

/**
 * @brief Set maximum samples
 * 
 * @param config Configuration to modify
 * @param max_samples Maximum samples (0 = unlimited)
 */
DDS_CONFIG_BUILDER_API void dds_config_set_max_samples(DDSConfig* config, int max_samples);

/* Logging Configuration Functions */

/**
 * @brief Set log level
 * 
 * @param config Configuration to modify
 * @param level Log level: 0=ERROR, 1=WARNING, 2=INFO, 3=DEBUG
 */
DDS_CONFIG_BUILDER_API void dds_config_set_log_level(DDSConfig* config, int level);

/**
 * @brief Set log file
 * 
 * @param config Configuration to modify
 * @param log_file Path to log file (NULL for stdout)
 */
DDS_CONFIG_BUILDER_API void dds_config_set_log_file(DDSConfig* config, const char* log_file);

/**
 * @brief Set participant name
 * 
 * @param config Configuration to modify
 * @param name Participant name
 */
DDS_CONFIG_BUILDER_API void dds_config_set_participant_name(DDSConfig* config, const char* name);

/**
 * @brief Disable topic name validation
 * 
 * WARNING: This allows non-standard topic names (e.g., starting with digits)
 * which violate DDS IDL specification. Use only in closed/private systems.
 * 
 * @param config Configuration to modify
 * @param disable 1 to disable validation, 0 to enable (default)
 */
DDS_CONFIG_BUILDER_API void dds_config_disable_topic_name_validation(DDSConfig* config, int disable);

/* ============================================================================
 * Configuration File Loader
 * ============================================================================
 * Load DDS configuration from INI-style configuration files with profiles.
 * 
 * Configuration File Format:
 * ```ini
 * [global]
 * service_url = http://localhost:9090
 * log_level = info
 * discovery_enabled = true    # 全局默认值
 * monitor_enabled = true      # 全局默认值
 * 
 * [default]
 * domain_id = 0
 * bind_address = 192.168.1.100
 * # 如果不指定 discovery_enabled/monitor_enabled，则使用 global 的值
 * ```
 * 
 * Priority: Environment Variables > Profile Config > Global Config > Defaults
 */

/**
 * @brief Global configuration structure (applies to all profiles)
 */
typedef struct {
    char service_url[256];      /**< Unified service URL (Discovery + Monitor) */
    char log_level[32];         /**< Log level: debug/info/warn/error */
    char log_file[512];         /**< Log file path (empty = console output) */
    int discovery_enabled;      /**< Enable dynamic discovery (default for all profiles) */
    int monitor_enabled;        /**< Enable monitoring (default for all profiles) */
} GlobalConfig;

/**
 * @brief Configuration loading options
 */
typedef struct {
    int use_env_override;       /**< Use environment variables to override (default: true) */
    int create_if_not_exist;    /**< Create config file if not found (default: true) */
    int verbose;                /**< Print detailed loading info (default: false) */
} ConfigLoadOptions;

/**
 * @brief Get default global configuration
 * 
 * @param config Global config structure to initialize
 */
DDS_CONFIG_BUILDER_API void dds_get_default_global_config(GlobalConfig* config);

/**
 * @brief Load configuration from file with specified profile
 * 
 * @param filepath Configuration file path
 * @param profile_name Profile name (e.g., "default", "production")
 * @param global_out Output global config (can be NULL)
 * @return DDSConfig* on success, NULL on failure (must free with dds_config_destroy)
 */
DDS_CONFIG_BUILDER_API DDSConfig* dds_config_load_from_file(const char* filepath, 
                                                             const char* profile_name,
                                                             GlobalConfig* global_out);

/**
 * @brief Load configuration from default path (./dds_config.ini)
 * 
 * @param profile_name Profile name to load
 * @param global_out Output global config (can be NULL)
 * @return DDSConfig* on success, NULL on failure
 */
DDS_CONFIG_BUILDER_API DDSConfig* dds_config_load_profile(const char* profile_name,
                                                           GlobalConfig* global_out);

/**
 * @brief Auto-load configuration (checks DDS_PROFILE environment variable)
 * 
 * Loading order:
 * 1. Check DDS_PROFILE env var for profile name
 * 2. Try to load from ./dds_config.ini
 * 3. Create default config file if not found
 * 4. Fall back to hardcoded defaults
 * 
 * @param global_out Output global config (can be NULL)
 * @return DDSConfig* (never NULL, returns defaults on failure)
 */
DDS_CONFIG_BUILDER_API DDSConfig* dds_config_load_auto(GlobalConfig* global_out);

/**
 * @brief Load configuration with custom options
 * 
 * @param profile_name Profile name to load
 * @param options Loading options
 * @param global_out Output global config (can be NULL)
 * @return DDSConfig* configuration object
 */
DDS_CONFIG_BUILDER_API DDSConfig* dds_config_load_with_options(const char* profile_name,
                                                                const ConfigLoadOptions* options,
                                                                GlobalConfig* global_out);

/**
 * @brief Create default configuration file at specified path
 * 
 * Creates a config file with example profiles:
 * - [global]: Global settings
 * - [default]: Local development
 * - [production]: Production environment
 * - [test]: Testing with minimal services
 * - [cross_machine]: Distributed deployment
 * 
 * @param filepath Path to create config file
 * @return 1 on success, 0 on failure
 */
DDS_CONFIG_BUILDER_API int dds_create_default_config_file(const char* filepath, const char* service_url);

/**
 * @brief Print global configuration (for debugging)
 * 
 * @param config Global config to print
 */
DDS_CONFIG_BUILDER_API void dds_print_global_config(const GlobalConfig* config);

/**
 * @brief Print complete configuration (global + DDS)
 * 
 * @param global Global configuration
 * @param dds DDS configuration
 */
DDS_CONFIG_BUILDER_API void dds_print_full_config(const GlobalConfig* global, const DDSConfig* dds);

/* Monitor Configuration Functions */

/**
 * @brief Enable or disable monitoring
 * 
 * @param config Configuration to modify
 * @param enable 1 to enable, 0 to disable
 */
DDS_CONFIG_BUILDER_API void dds_config_enable_monitor(DDSConfig* config, int enable);

/**
 * @brief Set monitor server URL
 * 
 * @param config Configuration to modify
 * @param url Monitor server URL
 */
DDS_CONFIG_BUILDER_API void dds_config_set_monitor_server_url(DDSConfig* config, const char* url);

/**
 * @brief Set monitor report interval
 * 
 * @param config Configuration to modify
 * @param interval_sec Interval in seconds
 */
DDS_CONFIG_BUILDER_API void dds_config_set_monitor_interval(DDSConfig* config, int interval_sec);

/**
 * @brief Set monitor application ID
 * 
 * @param config Configuration to modify
 * @param app_id Application identifier
 */
DDS_CONFIG_BUILDER_API void dds_config_set_monitor_app_id(DDSConfig* config, const char* app_id);

/**
 * @brief Set discovery initial peers
 * 
 * @param config Configuration to modify
 * @param peers Comma-separated peer addresses
 */
DDS_CONFIG_BUILDER_API void dds_config_set_discovery_initial_peers(DDSConfig* config, const char* peers);

#ifdef __cplusplus
}
#endif

#endif // DDS_CONFIG_BUILDER_H
