/**
 * @file situationforwarder.h
 * @brief 态势数据转发器（CPSS 收到的数据 -> RTI DDS 发布给态势显示）
 * @details 单例。Redis/FastDDS 等适配器收到数据后，调用 forwardPlatform() 把
 *          PlatformData 通过 RTI Connext DDS 发布出去，供外部态势显示系统订阅。
 *
 *          通过宏 CPSS_ENABLE_RTI_DDS 控制是否编译真实 RTI 代码。
 *          未安装 RTI Connext DDS 时为空实现桩，不影响工程编译。
 * @date 2026-11-22
 */

#ifndef SITUATIONFORWARDER_H
#define SITUATIONFORWARDER_H

#include "dynamicdata.h"
#include <QObject>

/**
 * @class SituationForwarder
 * @brief 态势数据转发单例（RTI DDS 发布端）
 */
class SituationForwarder : public QObject
{
    Q_OBJECT

public:
    static SituationForwarder* instance();

    /** @brief 初始化 RTI DDS 发布端 */
    bool initialize();

    /** @brief 关闭 */
    void shutdown();

    /** @brief 是否已启用真实 RTI DDS */
    bool isEnabled() const;

    /** @brief 转发单个平台数据到态势显示 */
    void forwardPlatform(const PlatformData &platform);

    /** @brief 转发多个平台数据 */
    void forwardPlatforms(const QList<PlatformData> &platforms);

private:
    explicit SituationForwarder(QObject *parent = nullptr);
    ~SituationForwarder();

    static SituationForwarder *s_instance;
    bool m_initialized = false;
};

#endif // SITUATIONFORWARDER_H
