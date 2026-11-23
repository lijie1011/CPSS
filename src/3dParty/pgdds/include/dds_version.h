/**
 * @file dds_version.h
 * @brief DDS SDK Version Information
 */

#ifndef DDS_VERSION_H
#define DDS_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

/* Version Components */
#define DDS_SDK_VERSION_MAJOR 1
#define DDS_SDK_VERSION_MINOR 0
#define DDS_SDK_VERSION_PATCH 7

/* Version String */
#define DDS_SDK_VERSION "1.0.7"

/* DDS Implementation - Set by CMake */
#ifndef DDS_IMPLEMENTATION
#define DDS_IMPLEMENTATION "?"
#endif

/* Full Version String with Implementation */
#define DDS_SDK_FULL_VERSION "DDS-SDK v" DDS_SDK_VERSION "-" DDS_IMPLEMENTATION

/**
 * @brief Get SDK version string
 * @return Version string (e.g., "1.0.0")
 */
const char* dds_sdk_get_version(void);

/**
 * @brief Get SDK implementation identifier
 * @return Implementation identifier (e.g., "C" for CycloneDDS, "F" for FastDDS)
 */
const char* dds_sdk_get_implementation(void);

/**
 * @brief Get full version string with implementation
 * @return Full version string (e.g., "DDS-SDK v1.0.0-C")
 */
const char* dds_sdk_get_full_version(void);

#ifdef __cplusplus
}
#endif

#endif /* DDS_VERSION_H */
