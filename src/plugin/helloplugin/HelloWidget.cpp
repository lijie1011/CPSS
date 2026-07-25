#include "HelloWidget.h"

HelloWidget::HelloWidget(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #f5f5f5;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setAlignment(Qt::AlignCenter);

    QLabel *helloLabel = new QLabel("<h1>Hello!</h1>", this);
    helloLabel->setStyleSheet("font-size: 48px; font-weight: bold; color: #333; text-align: center;");
    helloLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(helloLabel);

    QLabel *descLabel = new QLabel("This is a simple hello plugin.", this);
    descLabel->setStyleSheet("font-size: 16px; color: #666; text-align: center; margin-top: 20px;");
    descLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(descLabel);
}

HelloWidget::~HelloWidget()
{
}