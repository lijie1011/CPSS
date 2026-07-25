#ifndef SAMPLEWIDGET_H
#define SAMPLEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class SampleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SampleWidget(QWidget *parent = nullptr);
    ~SampleWidget();
};

#endif