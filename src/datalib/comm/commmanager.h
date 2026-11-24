/**
 * @file commmanager.h
 * @brief 通信管理器（统一装配所有协议适配器）
 * @details 单例。负责创建/启动/停止 HTTP、DDS 等协议适配器，
 *          并把它们统一注册到 DataManager。
 *          在 main.cpp 中调用 CommManager::instance()->startAll() 即可。
 * @date 2026-11-22
 */

#ifndef COMMMANAGER_H
#define COMMMANAGER_H

#include "protocoladapter.h"
#include <QObject>
#include <QList>

class IProtocolAdapter;

/**
 * @class CommManager
 * @brief 通信管理器单例，集中管理所有协议适配器
 */
class CPSS_DATA_API CommManager : public QObject
{
    Q_OBJECT

public:
    static CommManager* instance();

    /** @brief 创建并启动全部启用的适配器 */
    void startAll();

    /** @brief 停止全部适配器 */
    void stopAll();

    /** @brief 获取指定类型的适配器（可能为 nullptr） */
    IProtocolAdapter* adapter(ProtocolType type) const;

    /** @brief 获取所有适配器（供录制原始未融合数据等场景） */
    QList<IProtocolAdapter*> adapters() const { return m_adapters; }

    /**
     * @brief 通过 DDS 发布平台数据（供外部模块调用）
     */
    bool publishPlatformViaDds(const PlatformData &platform);

private:
    explicit CommManager(QObject *parent = nullptr);
    ~CommManager();

    static CommManager *s_instance;
    QList<IProtocolAdapter*> m_adapters;
    bool m_started = false;
};

#endif // COMMMANAGER_H
