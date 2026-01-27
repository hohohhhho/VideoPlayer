#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>

class ProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressBar(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent* ev)override;
    void enterEvent(QEnterEvent* ev)override;
    void leaveEvent(QEvent* ev)override;
    void mousePressEvent(QMouseEvent* ev)override;
    void mouseMoveEvent(QMouseEvent *ev)override;

    void setProgress(double progress);
    double getProgress(){return progress;};

    QColor color=Qt::gray;


signals:
    void changedProgress(double progress);
private:
    double progress=0;

    const int max_pen_size=9;
    const int min_pen_size=5;
    int current_pen_size=min_pen_size;
};

#endif // PROGRESSBAR_H
