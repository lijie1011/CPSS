#ifndef HELLOWIDGET_H
#define HELLOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class HelloWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HelloWidget(QWidget *parent = nullptr);
    ~HelloWidget();
};

#endif