/**
 * @file dds_nonfixed.h
 * @brief Non-fixed Topic Application Development Interface for DDS SDK
 * 
 * This header defines the C interface for non-fixed topic DDS operations
 * based on FastDDS implementation.
 */

#ifndef DDS_NONFIXED_H
#define DDS_NONFIXED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "dds_config.h"

/* DLL export/import macros */
#ifdef _WIN32
    #ifdef DDS_NONFIXED_EXPORTS
        #define DDS_NONFIXED_API __declspec(dllexport)
    #else
        #define DDS_NONFIXED_API __declspec(dllimport)
    #endif
#else
    #define DDS_NONFIXED_API
#endif

/**
 * @brief Data Writer QoS structure
 */
typedef struct DataWriterQos {
    int reliability_kind;          // 0: BEST_EFFORT, 1: RELIABLE
    int durability_kind;           // 0: VOLATILE, 1: TRANSIENT_LOCAL, 2: TRANSIENT, 3: PERSISTENT
    int history_kind;              // 0: KEEP_LAST, 1: KEEP_ALL
    int history_depth;             // History depth for KEEP_LAST
    int ownership_kind;            // 0: SHARED, 1: EXCLUSIVE
    int ownership_strength;        // Ownership strength value
    int liveliness_kind;           // 0: AUTOMATIC, 1: MANUAL_BY_PARTICIPANT, 2: MANUAL_BY_TOPIC
    long long liveliness_lease_duration_ms;  // Liveliness lease duration in milliseconds
    long long deadline_period_ms;  // Deadline period in milliseconds
    long long latency_budget_ms;   // Latency budget in milliseconds
    int transport_priority;        // Transport priority
    long long lifespan_ms;         // Lifespan in milliseconds
    long long max_blocking_time_ms;  // CRITICAL: Max blocking time for RELIABLE mode (milliseconds)
                                      // 0 = non-blocking, -1 = infinite wait, >0 = timeout duration
                                      // Default: 100ms (was hardcoded 10000ms causing 20s delays!)
    const char* partition;         // Partition name
    void* user_data;               // User data
    int user_data_length;          // User data length
} DataWriterQos;

/**
 * @brief Data Reader QoS structure
 */
typedef struct DataReaderQos {
    int reliability_kind;          // 0: BEST_EFFORT, 1: RELIABLE
    int durability_kind;           // 0: VOLATILE, 1: TRANSIENT_LOCAL, 2: TRANSIENT, 3: PERSISTENT
    int history_kind;              // 0: KEEP_LAST, 1: KEEP_ALL
    int history_depth;             // History depth for KEEP_LAST
    int ownership_kind;            // 0: SHARED, 1: EXCLUSIVE
    int liveliness_kind;           // 0: AUTOMATIC, 1: MANUAL_BY_PARTICIPANT, 2: MANUAL_BY_TOPIC
    long long liveliness_lease_duration_ms;  // Liveliness lease duration in milliseconds
    long long deadline_period_ms;  // Deadline period in milliseconds
    long long time_based_filter_ms; // Time-based filter in milliseconds
    const char* partition;         // Partition name
    void* user_data;               // User data
    int user_data_length;          // User data length
} DataReaderQos;

/**
 * @brief QoS Preset Enumeration
 * 
 * Predefined QoS profiles for common use cases to simplify configuration.
 * Each preset configures reliability_kind, durability_kind, history_kind, and history_depth.
 */
typedef enum QosPreset {
    /**
     * Best-Effort Mode (低延迟，尽力传输，不保证送达)
     * Use case: High-frequency sensor data, video streams where latest data matters most
     * Behavior:
     *   - Messages may be lost (no retransmission)
     *   - Lowest latency and overhead
     *   - Suitable for periodic data where only latest value matters
     * Settings:
     *   - reliability_kind = 0 (BEST_EFFORT)
     *   - durability_kind = 0 (VOLATILE)
     *   - history_kind = 0 (KEEP_LAST)
     *   - history_depth = 1
     */
    QOS_PRESET_BEST_EFFORT = 0,
    
    /**
     * Reliable Mode (可靠传输，保证在线订阅者必达，不保留历史)
     * Use case: Commands, events, notifications that must be delivered to active subscribers
     * Behavior:
     *   - Guaranteed delivery to online subscribers (with retransmission)
     *   - Late-joining subscribers will NOT receive historical messages
     *   - Buffers 10 samples for reliable retransmission
     *   - Suitable for real-time events and commands
     * Settings:
     *   - reliability_kind = 1 (RELIABLE)
     *   - durability_kind = 0 (VOLATILE) - No history retention for late-joiners
     *   - history_kind = 0 (KEEP_LAST)
     *   - history_depth = 10 - Prevents data loss during network congestion
     *   - max_blocking_time_ms = 5000 - Wait up to 5 seconds for send completion
     */
    QOS_PRESET_RELIABLE = 1
} QosPreset;

/**
 * @brief Message metadata information (extensible design)
 * 
 * Contains all metadata about a received message. Future fields can be added
 * without breaking API compatibility.
 * 
 * Note: reception_timestamp is approximated using dds_time() at callback entry.
 * For more accurate latency measurement, consider using external high-resolution timers.
 */
typedef struct MessageInfo {
    // Timestamp information (nanoseconds since 1970-01-01 UTC)
    int64_t source_timestamp;        // Sender's write timestamp (from DDS)
    int64_t reception_timestamp;     // Approximate receiver timestamp (using dds_time())
    
    // Sample state information
    uint32_t sample_state;           // READ (1) / NOT_READ (2)
    uint32_t view_state;             // NEW (1) / NOT_NEW (2)
    uint32_t instance_state;         // ALIVE (1) / NOT_ALIVE_DISPOSED (2) / NOT_ALIVE_NO_WRITERS (4)
    
    // QoS-related counters
    int32_t disposed_generation_count;
    int32_t no_writers_generation_count;
    
    // Sample ordering information
    int32_t sample_rank;             // Sample rank within instance
    int32_t generation_rank;         // Generation rank
    int32_t absolute_generation_rank;
    
    // Validity flags
    uint32_t valid_data;             // 1 if data is valid, 0 if metadata-only
    
    // Reserved for future extensions (do not use)
    void* reserved[7];
} MessageInfo;

/**
 * @brief Data Reader Listener callback function type (basic version)
 * 
 * @param domainId Domain identifier where data was received
 * @param topicName Topic identifier
 * @param buff Buffer containing received data
 * @param length Length of received data
 * @return 1 on success, 0 on failure
 */
typedef int (*DRListener)(int domainId, const char* topicName, char* buff, int length);

/**
 * @brief Extended Data Reader Listener callback with metadata (recommended)
 * 
 * This extended callback provides access to message timestamps and other metadata.
 * Use this for applications that need:
 * - End-to-end latency measurement
 * - Message ordering analysis
 * - Sample state inspection
 * 
 * @param domainId Domain identifier where data was received
 * @param topicName Topic identifier
 * @param buff Buffer containing received data
 * @param length Length of received data
 * @param info Pointer to MessageInfo structure containing metadata
 * @return 1 on success, 0 on failure
 * 
 * Example:
 * @code
 * int on_message(int domainId, const char* topicName, char* buff, int length, const MessageInfo* info) {
 *     // Calculate end-to-end latency
 *     int64_t latency_ns = info->reception_timestamp - info->source_timestamp;
 *     double latency_ms = latency_ns / 1000000.0;
 *     printf("Latency: %.2f ms\n", latency_ms);
 *     return 1;
 * }
 * @endcode
 */
typedef int (*DRListenerEx)(int domainId, const char* topicName, char* buff, int length, const MessageInfo* info);

/**
 * @brief Initialize DDS communication environment with specified domain ID
 * 
 * @param domainId Domain identifier
 * @return 1 on success, 0 on failure
 */
DDS_NONFIXED_API int nonfixed_init(int domainId);

/**
 * @brief Initialize DDS communication environment with configuration
 * 
 * Advanced initialization function that accepts a configuration structure
 * allowing programmatic control of network interfaces, discovery, transport,
 * and other DDS settings without XML files.
 * 
 * @param config Pointer to DDSConfig structure with desired settings
 * @return 1 on success, 0 on failure
 * 
 * @see dds_config.h and dds_config_builder.h for configuration options
 */
DDS_NONFIXED_API int nonfixed_init_with_config(DDSConfig* config);

/**
 * @brief Cleanup DDS communication environment and release resources
 * 
 * @param domainId Domain identifier
 * @return 1 on success, 0 on failure
 */
DDS_NONFIXED_API int nonfixed_cleanup(int domainId);

/**
 * @brief Publish a topic with specified QoS policies
 * 
 * @param domainId Domain identifier where topic will be published
 * @param topicName Topic identifier to publish
 * @param dwQos Data Writer QoS policy structure (NULL for default)
 * @param puber Publisher name
 * @return 1 on success, 0 on failure
 */
DDS_NONFIXED_API int nonfixed_pub_topic(int domainId, const char* topicName, DataWriterQos* dwQos, const char* puber);

/**
 * @brief Unpublish a previously published topic
 * 
 * @param domainId Domain identifier where topic was published
 * @param topicName Topic identifier to unpublish
 * @param puber Publisher name
 * @return 1 on success, 0 on failure
 */
DDS_NONFIXED_API int nonfixed_unpub_topic(int domainId, const char* topicName, const char* puber);

/**
 * @brief Send data for a published topic
 * 
 * @param domainId Domain identifier where topic is published
 * @param topicName Topic identifier
 * @param buffer Data buffer to send
 * @param length Length of data to send
 * @param puber Publisher name
 * @return 1 on success, 0 on failure
 */
DDS_NONFIXED_API int nonfixed_send_data(int domainId, const char* topicName, char* buffer, int length, const char* puber);

/**
 * @brief Subscribe to a topic with specified QoS policies and listener
 * 
 * @param domainId Domain identifier where topic will be subscribed
 * @param topicName Topic identifier to subscribe
 * @param drListener Listener callback function pointer (NULL for no callback)
 * @param drQos Data Reader QoS policy structure (NULL for default)
 * @param suber Subscriber name
 * @return 1 on success, 0 on failure
 */
DDS_NONFIXED_API int nonfixed_sub_topic(int domainId, const char* topicName, DRListener drListener, 
              DataReaderQos* drQos, const char* suber);

/**
 * @brief Subscribe to a topic with extended listener (includes timestamps and metadata)
 * 
 * This is the recommended subscription function for new code that needs access to
 * message metadata such as timestamps for latency measurement.
 * 
 * @param domainId Domain identifier where topic will be subscribed
 * @param topicName Topic identifier to subscribe
 * @param drListener Extended listener callback with MessageInfo parameter
 * @param drQos Data Reader QoS policy structure (NULL for default)
 * @param suber Subscriber name
 * @return 1 on success, 0 on failure
 * 
 * Example:
 * @code
 * int on_data_with_info(int domainId, const char* topic, char* data, int len, const MessageInfo* info) {
 *     int64_t latency = info->reception_timestamp - info->source_timestamp;
 *     printf("Received %d bytes with latency %lld ns\n", len, latency);
 *     return 1;
 * }
 * 
 * DataReaderQos* qos = create_datareader_qos_with_preset(QOS_PRESET_RELIABLE);
 * nonfixed_sub_topic_ex(0, "MyTopic", on_data_with_info, qos, "subscriber");
 * free_datareader_qos(qos);
 * @endcode
 */
DDS_NONFIXED_API int nonfixed_sub_topic_ex(int domainId, const char* topicName, DRListenerEx drListener, 
              DataReaderQos* drQos, const char* suber);

/**
 * @brief Subscribe to a topic with auto-matched QoS (simplified API)
 * 
 * This function discovers the publisher's QoS for the specified topic and
 * automatically creates a compatible subscriber. It also prints the discovered
 * QoS settings to stdout for debugging purposes.
 * 
 * Use this when you don't know the publisher's QoS settings and want the SDK
 * to automatically match them.
 * 
 * @param domainId Domain identifier where topic will be subscribed
 * @param topicName Topic identifier to subscribe
 * @param drListener Listener callback function pointer (NULL for no callback)
 * @param suber Subscriber name
 * @return 1 on success, 0 on failure (including if topic not found)
 * 
 * Example:
 * @code
 * // Simple subscription without specifying QoS
 * if (nonfixed_sub_topic_auto(0, "SensorData", on_data_received, "my_subscriber")) {
 *     printf("Subscribed with auto-matched QoS\n");
 * }
 * @endcode
 */
DDS_NONFIXED_API int nonfixed_sub_topic_auto(int domainId, const char* topicName, 
              DRListener drListener, const char* suber);

/**
 * @brief Unsubscribe from a previously subscribed topic
 * 
 * @param domainId Domain identifier where topic was subscribed
 * @param topicName Topic identifier to unsubscribe
 * @param suber Subscriber name
 * @return 1 on success, 0 on failure
 */
DDS_NONFIXED_API int nonfixed_unsub_topic(int domainId, const char* topicName, const char* suber);

/**
 * @brief Create Data Writer QoS with preset configuration
 * 
 * Simplifies QoS configuration by providing predefined profiles for common scenarios.
 * Recommended for new code to avoid manual QoS field configuration.
 * 
 * @param preset QoS preset enumeration (QOS_PRESET_BEST_EFFORT, QOS_PRESET_RELIABLE, etc.)
 * @return Pointer to DataWriterQos structure configured with preset values
 * 
 * Example:
 * @code
 * DataWriterQos* qos = create_datawriter_qos_with_preset(QOS_PRESET_SENSOR);
 * nonfixed_pub_topic("SensorData", qos, "sensor_publisher");
 * free_datawriter_qos(qos);
 * @endcode
 */
DDS_NONFIXED_API DataWriterQos* create_datawriter_qos_with_preset(int preset);

/**
 * @brief Create Data Reader QoS with preset configuration
 * 
 * Simplifies QoS configuration by providing predefined profiles for common scenarios.
 * Must use matching preset with corresponding DataWriter for successful communication.
 * 
 * @param preset QoS preset enumeration (QOS_PRESET_BEST_EFFORT, QOS_PRESET_RELIABLE, etc.)
 * @return Pointer to DataReaderQos structure configured with preset values
 * 
 * Example:
 * @code
 * DataReaderQos* qos = create_datareader_qos_with_preset(QOS_PRESET_SENSOR);
 * nonfixed_sub_topic("SensorData", qos, NULL, "sensor_subscriber");
 * free_datareader_qos(qos);
 * @endcode
 */
DDS_NONFIXED_API DataReaderQos* create_datareader_qos_with_preset(int preset);

/**
 * @brief Helper function to create default Data Writer QoS
 * 
 * @return Pointer to DataWriterQos structure with default values
 */
DDS_NONFIXED_API DataWriterQos* create_default_datawriter_qos(void);

/**
 * @brief Helper function to create default Data Reader QoS
 * 
 * @return Pointer to DataReaderQos structure with default values
 */
DDS_NONFIXED_API DataReaderQos* create_default_datareader_qos(void);

/**
 * @brief Free Data Writer QoS structure
 * 
 * @param qos Pointer to DataWriterQos to free
 */
DDS_NONFIXED_API void free_datawriter_qos(DataWriterQos* qos);

/**
 * @brief Free Data Reader QoS structure
 * 
 * @param qos Pointer to DataReaderQos to free
 */
DDS_NONFIXED_API void free_datareader_qos(DataReaderQos* qos);

/* ========== QoS Setter Functions ========== */

/**
 * @brief Set reliability kind for DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @param kind 0: BEST_EFFORT, 1: RELIABLE
 */
DDS_NONFIXED_API void datawriter_qos_set_reliability(DataWriterQos* qos, int kind);

/**
 * @brief Set durability kind for DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @param kind 0: VOLATILE, 1: TRANSIENT_LOCAL, 2: TRANSIENT, 3: PERSISTENT
 */
DDS_NONFIXED_API void datawriter_qos_set_durability(DataWriterQos* qos, int kind);

/**
 * @brief Set history policy for DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @param kind 0: KEEP_LAST, 1: KEEP_ALL
 * @param depth History depth (used when kind is KEEP_LAST)
 */
DDS_NONFIXED_API void datawriter_qos_set_history(DataWriterQos* qos, int kind, int depth);

/**
 * @brief Set ownership kind for DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @param kind 0: SHARED, 1: EXCLUSIVE
 */
DDS_NONFIXED_API void datawriter_qos_set_ownership(DataWriterQos* qos, int kind);

/**
 * @brief Set ownership strength for DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @param strength Ownership strength value
 */
DDS_NONFIXED_API void datawriter_qos_set_ownership_strength(DataWriterQos* qos, int strength);

/**
 * @brief Set liveliness policy for DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @param kind 0: AUTOMATIC, 1: MANUAL_BY_PARTICIPANT, 2: MANUAL_BY_TOPIC
 * @param lease_duration_ms Lease duration in milliseconds
 */
DDS_NONFIXED_API void datawriter_qos_set_liveliness(DataWriterQos* qos, int kind, long long lease_duration_ms);

/**
 * @brief Set deadline period for DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @param period_ms Deadline period in milliseconds
 */
DDS_NONFIXED_API void datawriter_qos_set_deadline(DataWriterQos* qos, long long period_ms);

/**
 * @brief Set latency budget for DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @param latency_ms Latency budget in milliseconds
 */
DDS_NONFIXED_API void datawriter_qos_set_latency_budget(DataWriterQos* qos, long long latency_ms);

/**
 * @brief Set transport priority for DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @param priority Transport priority value
 */
DDS_NONFIXED_API void datawriter_qos_set_transport_priority(DataWriterQos* qos, int priority);

/**
 * @brief Set lifespan for DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @param lifespan_ms Lifespan in milliseconds
 */
DDS_NONFIXED_API void datawriter_qos_set_lifespan(DataWriterQos* qos, long long lifespan_ms);

/**
 * @brief Set reliability kind for DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @param kind 0: BEST_EFFORT, 1: RELIABLE
 */
DDS_NONFIXED_API void datareader_qos_set_reliability(DataReaderQos* qos, int kind);

/**
 * @brief Set durability kind for DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @param kind 0: VOLATILE, 1: TRANSIENT_LOCAL, 2: TRANSIENT, 3: PERSISTENT
 */
DDS_NONFIXED_API void datareader_qos_set_durability(DataReaderQos* qos, int kind);

/**
 * @brief Set history policy for DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @param kind 0: KEEP_LAST, 1: KEEP_ALL
 * @param depth History depth (used when kind is KEEP_LAST)
 */
DDS_NONFIXED_API void datareader_qos_set_history(DataReaderQos* qos, int kind, int depth);

/**
 * @brief Set ownership kind for DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @param kind 0: SHARED, 1: EXCLUSIVE
 */
DDS_NONFIXED_API void datareader_qos_set_ownership(DataReaderQos* qos, int kind);

/**
 * @brief Set liveliness policy for DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @param kind 0: AUTOMATIC, 1: MANUAL_BY_PARTICIPANT, 2: MANUAL_BY_TOPIC
 * @param lease_duration_ms Lease duration in milliseconds
 */
DDS_NONFIXED_API void datareader_qos_set_liveliness(DataReaderQos* qos, int kind, long long lease_duration_ms);

/**
 * @brief Set deadline period for DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @param period_ms Deadline period in milliseconds
 */
DDS_NONFIXED_API void datareader_qos_set_deadline(DataReaderQos* qos, long long period_ms);

/**
 * @brief Set time-based filter for DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @param filter_ms Time-based filter in milliseconds
 */
DDS_NONFIXED_API void datareader_qos_set_time_based_filter(DataReaderQos* qos, long long filter_ms);

/* ========== QoS Getter Functions ========== */

/**
 * @brief Get reliability kind from DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @return Reliability kind (0: BEST_EFFORT, 1: RELIABLE)
 */
DDS_NONFIXED_API int datawriter_qos_get_reliability(const DataWriterQos* qos);

/**
 * @brief Get durability kind from DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @return Durability kind
 */
DDS_NONFIXED_API int datawriter_qos_get_durability(const DataWriterQos* qos);

/**
 * @brief Get history kind from DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @return History kind (0: KEEP_LAST, 1: KEEP_ALL)
 */
DDS_NONFIXED_API int datawriter_qos_get_history_kind(const DataWriterQos* qos);

/**
 * @brief Get history depth from DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @return History depth
 */
DDS_NONFIXED_API int datawriter_qos_get_history_depth(const DataWriterQos* qos);

/**
 * @brief Get ownership kind from DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @return Ownership kind (0: SHARED, 1: EXCLUSIVE)
 */
DDS_NONFIXED_API int datawriter_qos_get_ownership(const DataWriterQos* qos);

/**
 * @brief Get liveliness kind from DataWriter QoS
 * @param qos Pointer to DataWriterQos
 * @return Liveliness kind
 */
DDS_NONFIXED_API int datawriter_qos_get_liveliness(const DataWriterQos* qos);

/**
 * @brief Get reliability kind from DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @return Reliability kind (0: BEST_EFFORT, 1: RELIABLE)
 */
DDS_NONFIXED_API int datareader_qos_get_reliability(const DataReaderQos* qos);

/**
 * @brief Get durability kind from DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @return Durability kind
 */
DDS_NONFIXED_API int datareader_qos_get_durability(const DataReaderQos* qos);

/**
 * @brief Get history kind from DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @return History kind (0: KEEP_LAST, 1: KEEP_ALL)
 */
DDS_NONFIXED_API int datareader_qos_get_history_kind(const DataReaderQos* qos);

/**
 * @brief Get history depth from DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @return History depth
 */
DDS_NONFIXED_API int datareader_qos_get_history_depth(const DataReaderQos* qos);

/**
 * @brief Get ownership kind from DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @return Ownership kind (0: SHARED, 1: EXCLUSIVE)
 */
DDS_NONFIXED_API int datareader_qos_get_ownership(const DataReaderQos* qos);

/**
 * @brief Get liveliness kind from DataReader QoS
 * @param qos Pointer to DataReaderQos
 * @return Liveliness kind
 */
DDS_NONFIXED_API int datareader_qos_get_liveliness(const DataReaderQos* qos);

/* ============================================================================
 * Topic Discovery API
 * ============================================================================ */

/**
 * @brief Opaque discovery context handle
 */
typedef struct dds_discovery_context dds_discovery_context_t;

/**
 * @brief Discovered endpoint QoS information
 */
typedef struct DiscoveredQos {
    int32_t reliability_kind;    /* 0=BEST_EFFORT, 1=RELIABLE */
    int32_t durability_kind;     /* 0=VOLATILE, 1=TRANSIENT_LOCAL, 2=TRANSIENT, 3=PERSISTENT */
    int32_t history_kind;        /* 0=KEEP_LAST, 1=KEEP_ALL */
    int32_t history_depth;       /* KEEP_LAST depth */
    int32_t ownership_kind;      /* 0=SHARED, 1=EXCLUSIVE */
    int32_t ownership_strength;  /* EXCLUSIVE strength */
    int64_t deadline_ns;         /* Deadline period (nanoseconds), 0=infinite */
    int64_t lifespan_ns;         /* Lifespan (nanoseconds), 0=infinite */
    bool qos_valid;              /* QoS validity flag */
} DiscoveredQos;

/**
 * @brief Discovered topic information with network address
 */
typedef struct DiscoveredTopic {
    char topic_name[256];           /* Topic name */
    char type_name[256];            /* Data type name */
    int32_t writer_count;           /* Number of writers */
    int32_t reader_count;           /* Number of readers */
    
    /* Network address of latest publisher */
    char latest_publisher_address[64];
    uint32_t latest_publisher_port;
    
    /* QoS information (from first writer, if available) */
    DiscoveredQos writer_qos;
} DiscoveredTopic;

/**
 * @brief Topic discovery callback function
 * 
 * Called when a topic appears or disappears in the domain.
 * Note: This callback is invoked from DDS internal thread, keep processing minimal.
 * 
 * @param topic Topic information
 * @param appeared true if topic appeared, false if disappeared
 * @param user_data User-provided context data
 */
typedef void (*dds_discovery_topic_callback_t)(
    const DiscoveredTopic* topic,
    bool appeared,
    void* user_data
);

/**
 * @brief Create a topic discovery context
 * 
 * @param domain_id DDS domain ID
 * @param config_xml Optional XML configuration (can be NULL)
 * @return Discovery context handle, or NULL on failure
 */
DDS_NONFIXED_API dds_discovery_context_t* dds_discovery_create(
    int32_t domain_id,
    const char* config_xml
);

/**
 * @brief Destroy discovery context and free resources
 * 
 * Automatically stops async discovery if running.
 * 
 * @param ctx Discovery context
 */
DDS_NONFIXED_API void dds_discovery_destroy(dds_discovery_context_t* ctx);

/**
 * @brief Get domain ID from discovery context
 * 
 * @param ctx Discovery context
 * @return Domain ID, or -1 on error
 */
DDS_NONFIXED_API int32_t dds_discovery_get_domain_id(dds_discovery_context_t* ctx);

/**
 * @brief Set topic discovery callback for async notifications
 * 
 * The callback will be invoked when topics appear or disappear.
 * Must be called before dds_discovery_start_async().
 * 
 * @param ctx Discovery context
 * @param callback Callback function (can be NULL to disable)
 * @param user_data User context passed to callback
 */
DDS_NONFIXED_API void dds_discovery_set_topic_callback(
    dds_discovery_context_t* ctx,
    dds_discovery_topic_callback_t callback,
    void* user_data
);

/**
 * @brief Start asynchronous topic discovery
 * 
 * Enables automatic topic detection using DDS listeners.
 * Callbacks will be invoked when topics appear/disappear.
 * 
 * @param ctx Discovery context
 * @return 0 on success, negative on error
 */
DDS_NONFIXED_API int32_t dds_discovery_start_async(dds_discovery_context_t* ctx);

/**
 * @brief Stop asynchronous topic discovery
 * 
 * @param ctx Discovery context
 */
DDS_NONFIXED_API void dds_discovery_stop_async(dds_discovery_context_t* ctx);

/**
 * @brief Get library version string
 * 
 * @return Version string (e.g., "1.0.0")
 */
DDS_NONFIXED_API const char* dds_discovery_version(void);

#ifdef __cplusplus
}
#endif

#endif /* DDS_NONFIXED_H */
