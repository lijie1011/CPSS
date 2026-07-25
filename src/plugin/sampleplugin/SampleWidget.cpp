#include "SampleWidget.h"

SampleWidget::SampleWidget(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #f0f0f0;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLabel = new QLabel("<h2>Sample Plugin</h2>", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    layout->addWidget(titleLabel);

    QLabel *descLabel = new QLabel("This is a sample plugin demonstrating the CPSS plugin system.", this);
    descLabel->setStyleSheet("font-size: 14px; color: #666; margin-top: 10px;");
    layout->addWidget(descLabel);

    QLabel *infoLabel = new QLabel("<p><b>Features:</b></p>"
                                   "<ul>"
                                   "<li>Dynamic loading from DLL</li>"
                                   "<li>Button in status bar</li>"
                                   "<li>Widget displayed in stacked area</li>"
                                   "<li>Multiple plugins supported</li>"
                                   "</ul>", this);
    infoLabel->setStyleSheet("font-size: 13px; color: #555; margin-top: 15px;");
    layout->addWidget(infoLabel);

    layout->addStretch();
}

SampleWidget::~SampleWidget()
{
}