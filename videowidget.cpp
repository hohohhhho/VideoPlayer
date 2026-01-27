#include "videowidget.h"

#include <QPainter>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget{parent}
{}

void VideoWidget::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter painter(this);

    int widgetWidth = this->width();
    int widgetHeight = this->height();
    int videoWidth = pxp_frame.width();
    int videoHeight = pxp_frame.height();

    qreal widgetAspectRatio = static_cast<qreal>(widgetWidth) / widgetHeight;
    qreal videoAspectRatio = static_cast<qreal>(videoWidth) / videoHeight;

    QRect destRect;
    if (widgetAspectRatio > videoAspectRatio) {
        int scaledWidth = static_cast<int>(videoWidth * (widgetHeight / static_cast<qreal>(videoHeight)));
        destRect = QRect((widgetWidth - scaledWidth) / 2, 0, scaledWidth, widgetHeight);
    } else {
        int scaledHeight = static_cast<int>(videoHeight * (widgetWidth / static_cast<qreal>(videoWidth)));
        destRect = QRect(0, (widgetHeight - scaledHeight) / 2, widgetWidth, scaledHeight);
    }

    painter.drawPixmap(destRect, pxp_frame);

}

void VideoWidget::setFrame(const QVideoFrame& frame)
{
    this->pxp_frame=QPixmap::fromImage(frame.toImage());
    update();
}
