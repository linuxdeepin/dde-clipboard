#include "itemwidget.h"
#include "constants.h"
#include "clipboardmodel.h"

#include <QPainter>
#include <QTextOption>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QDir>
#include <QTemporaryFile>
#include <QApplication>

#include <DFontSizeManager>

static QPixmap GetFileIcon(QString path)
{
    QFileInfo fileInfo(path);
    QFileIconProvider icon;
    QPixmap pixmap = icon.icon(fileInfo).pixmap(128, 128);
    return pixmap;
}

QIcon fileIcon(const QString &extension)
{
    QFileIconProvider provider;
    QIcon icon;
    QString strTemplateName = QDir::tempPath() + QDir::separator() + "_XXXXXX." + extension;
    QTemporaryFile tmpFile(strTemplateName);
    tmpFile.setAutoRemove(false);

    if (tmpFile.open()) {
        tmpFile.close();
        icon = provider.icon(QFileInfo(strTemplateName));
        // tmpFile.remove();
    } else {
        qCritical() << QString("failed to write temporary file %1").arg(tmpFile.fileName());
    }

    return icon;
}

ItemTitle::ItemTitle(QWidget *parent)
    : DWidget(parent)
    , m_icon(new DIconButton(this))
    , m_nameLabel(new QLabel(this))
    , m_timeLabel(new QLabel(this))
    , m_refreshTimer(new QTimer(this))
    , m_closeButton(new DIconButton(DStyle::StandardPixmap::SP_CloseButton, this))
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(8);

    layout->addWidget(m_icon);
    layout->addWidget(m_nameLabel);
    layout->addWidget(m_timeLabel);
    layout->addWidget(m_closeButton);

    setFixedHeight(TitleHeight);
    m_icon->setFixedSize(TitleHeight, TitleHeight);
    m_icon->setFlat(true);

    QFont font = DFontSizeManager::instance()->t4();
    font.setWeight(75);
    m_nameLabel->setFont(font);
    m_nameLabel->setAlignment(Qt::AlignLeft);

    m_timeLabel->setAlignment(Qt::AlignRight);

    m_closeButton->setFlat(true);
    m_closeButton->setIconSize(QSize(TitleHeight / 2, TitleHeight / 2));
    m_closeButton->setFixedSize(TitleHeight, TitleHeight);
    m_closeButton->setVisible(false);

    m_refreshTimer->setInterval(60 * 1000);
    connect(m_refreshTimer, &QTimer::timeout, this, &ItemTitle::onRefreshTime);

    connect(m_closeButton, &DIconButton::clicked, this, &ItemTitle::close);

    setFocusPolicy(Qt::NoFocus);
}

void ItemTitle::setDataName(const QString &text)
{
    m_nameLabel->setText(text);
}

void ItemTitle::setIcon(const QIcon &icon)
{
    m_icon->setIcon(icon);
}

void ItemTitle::setCreateTime(const QDateTime &time)
{
    m_time = time;
    onRefreshTime();
}

void ItemTitle::onHoverStateChanged(bool hover)
{
    if (hover) {
        m_timeLabel->hide();
        m_closeButton->show();
    } else {
        m_timeLabel->show();
        m_closeButton->hide();
    }
}

void ItemTitle::onRefreshTime()
{
    m_timeLabel->setText(CreateTimeString(m_time));

    m_refreshTimer->stop();

    int interval;
    int minuteElapsed = m_time.secsTo(QDateTime::currentDateTime()) / 60;
    if (minuteElapsed < 60) {
        interval = 60 * 1000;
    } else {
        interval = 60 * 60 * 1000;
    }
    m_refreshTimer->start(interval);
}

QString ItemTitle::CreateTimeString(const QDateTime &time)
{
    QString text;

    QDateTime t = QDateTime::currentDateTime();

    if (time.secsTo(t) < 60) {//60秒以内
        text = tr("Now");
    } else if (time.secsTo(t) >= 60 && time.secsTo(t) < 2 * 60) { //一分钟
        text = tr("1 minute ago");
    } else if (time.secsTo(t) >= 2 * 60 && time.secsTo(t) < 60 * 60) { //一小时内
        text = QString::number(time.secsTo(t) / 60) + tr("minutes ago");
    } else if (time.secsTo(t) >= 60 * 60 && time.secsTo(t) < 2 * 60 * 60) {//两小时内
        text = tr("1 hour ago");
    } else if (time.secsTo(t) >= 2 * 60 * 60 && time.daysTo(t) < 1) { //今天凌晨0点以后
        text = QString::number(time.secsTo(t) / 60 / 60) + tr("hours ago");
    } else if (time.daysTo(t) >= 1 && time.daysTo(t) < 2) { //今天凌晨0点以前的
        text = tr("Yesterday") + time.toString(" hh:mm");
    } else if (time.daysTo(t) >= 2 && time.daysTo(t) < 7) { //昨天凌晨0点以前的
        text = time.toString("ddd hh:mm");
    } else if (time.daysTo(t) >= 7) { //一周前0点以前的
        text = time.toString("yyyy/MM/dd");
    }

    return text;
}

AlphaWidget::AlphaWidget(QWidget *parent)
    : DWidget(parent)
{
}

void AlphaWidget::setAlpha(int alpha)
{
    m_hoverAlpha = alpha;
    m_unHoverAlpha = alpha;

    update();
}

void AlphaWidget::setRadius(int radius)
{
    m_radius = radius;

    update();
}

void AlphaWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPalette pe = this->palette();
    QColor c = pe.color(QPalette::Base);

    QColor brushColor(c);
    brushColor.setAlpha(m_havor ? m_hoverAlpha : m_unHoverAlpha);
    painter.setBrush(brushColor);

    QPen borderPen;
    borderPen.setColor(Qt::transparent);
    painter.setPen(borderPen);
    painter.drawRoundRect(QRectF(0, 0, width(), height()), m_radius, m_radius);

    return QWidget::paintEvent(event);
}

void AlphaWidget::enterEvent(QEvent *event)
{
    m_havor = true;

    update();

    return DWidget::enterEvent(event);
}

void AlphaWidget::leaveEvent(QEvent *event)
{
    m_havor = false;

    update();

    return DWidget::leaveEvent(event);
}

ItemWidget::ItemWidget(ClipboardModel *model, QPointer<ItemData> data, QWidget *parent)
    : AlphaWidget(parent)
    , m_model(model)
    , m_data(data)
    , m_titleWidget(new ItemTitle(this))
    , m_contentLabel(new Dtk::Widget::DLabel(this))
    , m_statusLabel(new Dtk::Widget::DLabel(this))
    , m_layout(new QVBoxLayout(this))
    , m_board(QApplication::clipboard())
{
    initUI();
    initStyle(m_data);

    connect(this, &ItemWidget::hoverStateChanged, m_titleWidget, &ItemTitle::onHoverStateChanged);
    connect(m_titleWidget, &ItemTitle::close, this, [ = ] {
        m_model->removeData(m_data);
    });
    connect(this, &ItemWidget::clicked, this, &ItemWidget::onClicked);
}

void ItemWidget::onClicked()
{
    //剪切板上的内容不再进行复制
    if (m_model->data().indexOf(m_data) == 0) {
        return;
    }

    QMimeData *mimeData = new QMimeData;
    switch (m_data->type()) {
    case ItemData::Text:
        mimeData->setText(m_text);
        mimeData->setHtml(m_text);
        break;
    case ItemData::Image:
        mimeData->setImageData(m_pixmap);
        break;
    case ItemData::File:
        mimeData->setUrls(m_urls);
        break;
    default:
        break;
    }

    m_board->setMimeData(mimeData);
}

void ItemWidget::initUI()
{
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->addWidget(m_titleWidget, 0, Qt::AlignTop);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(21);
    layout->addWidget(m_contentLabel, 0, Qt::AlignTop);
    m_layout->addLayout(layout, 0);
    m_layout->addWidget(m_statusLabel, 0, Qt::AlignBottom);

    m_statusLabel->setFixedHeight(StatusBarHeight);

    m_contentLabel->setWordWrap(true);
    m_contentLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setAlignment(Qt::AlignCenter);

    setHoverAlpha(160);
    setUnHoverAlpha(80);
    setRadius(8);

    setFocusPolicy(Qt::NoFocus);
}

void ItemWidget::initStyle(QPointer<ItemData> data)
{
    setDataName(data->title());
    setIcon(data->icon());
    setCreateTime(data->createTime());

    switch (data->type()) {
    case ItemData::Text: {
        QFont font = m_contentLabel->font();
        font.setItalic(true);
        m_contentLabel->setFont(font);
        m_contentLabel->setAlignment(Qt::AlignTop);
        setText(data->contentText(), data->subTitle());
    }
    break;
    case ItemData::Image: {
        m_contentLabel->setAlignment(Qt::AlignCenter);
        setPixmap(data->contentImage());
    }
    break;
    case ItemData::File: {
        m_urls = data->urls();
        if (data->urls().size() == 0)
            return;

        QString first = data->urls().first().toString();
        if (data->urls().size() == 1) {
            QFileInfo info(first);
            m_statusLabel->setText(info.fileName());
        } else if (data->urls().size() > 1) {
            QFileInfo info(first);
            m_statusLabel->setText(info.fileName() + tr(" files(%2...)").arg(data->urls().size()));
        }

        setFilePixmap(GetFileIcon(first));
    }
    break;
    }
}

void ItemWidget::setText(const QString &text, const QString &length)
{
    m_text = text;

    m_contentLabel->setText(text);

    m_statusLabel->setText(length);
}

void ItemWidget::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    qreal scale = 1.0;
    if (pixmap.size() == QSize(0, 0))
        return;

    if (pixmap.width() >= pixmap.height()) {
        scale = pixmap.width() * 1.0 / 128;
    } else {
        scale = pixmap.height() * 1.0 / 210;
    }

    m_contentLabel->setPixmap(pixmap.scaled(pixmap.size() / scale, Qt::KeepAspectRatio));
    m_statusLabel->setText(QString::number(pixmap.width()) + "*" + QString::number(pixmap.height()) + tr("px"));
}

void ItemWidget::setFilePixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    qreal scale = 1.0;
    if (pixmap.size() == QSize(0, 0))
        return;

    if (pixmap.width() >= pixmap.height()) {
        scale = pixmap.width() * 1.0 / 128;
    } else {
        scale = pixmap.height() * 1.0 / 210;
    }

    m_contentLabel->setPixmap(pixmap.scaled(pixmap.size() / scale, Qt::KeepAspectRatio));
}

void ItemWidget::setDataName(const QString &text)
{
    return m_titleWidget->setDataName(text);
}

void ItemWidget::setIcon(const QIcon &icon)
{
    m_titleWidget->setIcon(icon);
}

void ItemWidget::setCreateTime(const QDateTime &time)
{
    m_titleWidget->setCreateTime(time);
}

void ItemWidget::mousePressEvent(QMouseEvent *event)
{
    Q_EMIT clicked();

    return AlphaWidget::mousePressEvent(event);
}

void ItemWidget::enterEvent(QEvent *event)
{
    Q_EMIT hoverStateChanged(true);

    return AlphaWidget::enterEvent(event);
}

void ItemWidget::leaveEvent(QEvent *event)
{
    Q_EMIT hoverStateChanged(false);

    return AlphaWidget::leaveEvent(event);
}

void ItemWidget::focusInEvent(QFocusEvent *event)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT hoverStateChanged(true);

    return AlphaWidget::focusInEvent(event);
}

void ItemWidget::focusOutEvent(QFocusEvent *event)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT hoverStateChanged(false);

    return AlphaWidget::focusOutEvent(event);
}
