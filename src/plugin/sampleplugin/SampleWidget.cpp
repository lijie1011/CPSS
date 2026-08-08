#include "SampleWidget.h"

SampleWidget::SampleWidget(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #f0f0f0;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLabel = new QLabel(QStringLiteral("<h2>示例插件</h2>"), this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    layout->addWidget(titleLabel);

    QLabel *descLabel = new QLabel(QStringLiteral("这是一个演示 CPSS 插件系统的示例插件。"), this);
    descLabel->setStyleSheet("font-size: 14px; color: #666; margin-top: 10px;");
    layout->addWidget(descLabel);

    QLabel *infoLabel = new QLabel(QStringLiteral("<p><b>功能特性:</b></p>"
                                   "<ul>"
                                   "<li>从 DLL 动态加载</li>"
                                   "<li>状态栏按钮</li>"
                                   "<li>嵌入布局的界面</li>"
                                   "<li>支持多插件共存</li>"
                                   "</ul>"), this);
    infoLabel->setStyleSheet("font-size: 13px; color: #555; margin-top: 15px;");
    layout->addWidget(infoLabel);

    layout->addStretch();
}

SampleWidget::~SampleWidget()
{
}