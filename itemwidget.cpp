#include "itemwidget.h"
#include "constants.h"
#include "pixmaplabel.h"

#include <QPainter>
#include <QTextOption>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QApplication>
#include <QMouseEvent>
#include <QDir>
#include <QTemporaryFile>
#include <QBitmap>
#include <QImageReader>
#include <QFileIconProvider>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

static QPixmap GetFileIcon(QString path)
{
    QPixmap pixmap;
    QFileIconProvider provider;
    QFileInfo fileInfo(path);
    if (fileInfo.suffix().isEmpty() && QDir(path).exists()) {
        pixmap = provider.icon(QFileIconProvider::Folder).pixmap(FileIconWidth, FileIconWidth);
    } else {
        pixmap = provider.icon(fileInfo).pixmap(FileIconWidth, FileIconWidth);
    }
    return pixmap;
}

ItemWidget::ItemWidget(QPointer<ItemData> data, QWidget *parent)
    : DWidget(parent)
    , m_data(data)
    , m_nameLabel(new QLabel(this))
    , m_timeLabel(new QLabel(this))
    , m_closeButton(new DIconButton(DStyle::StandardPixmap::SP_CloseButton, this))
    , m_contentLabel(new PixmapLabel/*Dtk::Widget::DLabel*/(this))
    , m_statusLabel(new Dtk::Widget::DLabel(this))
    , m_refreshTimer(new QTimer(this))
    , m_layout(new QVBoxLayout(this))
{
    initUI();
    initStyle(m_data);

    connect(m_refreshTimer, &QTimer::timeout, this, &ItemWidget::onRefreshTime);
    connect(this, &ItemWidget::hoverStateChanged, this, &ItemWidget::onHoverStateChanged);
    connect(m_closeButton, &DIconButton::clicked, [ = ] {
        m_data->remove();
    });
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        setPixmap(m_pixmap);
    });
}

void ItemWidget::setText(const QString &text, const QString &length)
{
    QFont font = m_contentLabel->font();
    font.setItalic(true);
    m_contentLabel->setFont(font);
    m_contentLabel->setText(text);

    m_statusLabel->setText(length);
}

void ItemWidget::setPixmap(const QPixmap &pixmap)
{
    if (!m_pixmap.isNull() && (m_data->type() == ItemData::Image
                               || m_data->type() == ItemData::File)) {
        QPixmap pix = Globals::GetRoundPixmap(pixmap, palette().color(QPalette::Base));
        m_contentLabel->setPixmap(Globals::pixmapScaled(pix));
        m_statusLabel->setText(QString::number(pixmap.width()) + "X" + QString::number(pixmap.height()) + tr("px"));
    }
}

void ItemWidget::setFilePixmap(const QPixmap &pixmap)
{
    qreal scale = Globals::GetScale(pixmap.size(), FileIconWidth, FileIconHeight);

    m_contentLabel->setPixmap(pixmap.scaled(pixmap.size() / scale, Qt::KeepAspectRatio));
}

void ItemWidget::setFilePixmaps(const QList<QPixmap> &list)
{
    m_contentLabel->setPixmapList(list);
}

void ItemWidget::setClipType(const QString &text)
{
    m_nameLabel->setText(text);
}

void ItemWidget::setCreateTime(const QDateTime &time)
{
    m_createTime = time;
    onRefreshTime();
}

void ItemWidget::setAlpha(int alpha)
{
    m_hoverAlpha = alpha;
    m_unHoverAlpha = alpha;

    update();
}

int ItemWidget::hoverAlpha() const
{
    return m_hoverAlpha;
}

void ItemWidget::setHoverAlpha(int alpha)
{
    m_hoverAlpha = alpha;

    update();
}

int ItemWidget::unHoverAlpha() const
{
    return m_unHoverAlpha;
}

void ItemWidget::setUnHoverAlpha(int alpha)
{
    m_unHoverAlpha = alpha;

    update();
}

void ItemWidget::setRadius(int radius)
{
    m_radius = radius;

    update();
}

void ItemWidget::onHoverStateChanged(bool hover)
{
    m_havor = hover;

    if (hover) {
        m_timeLabel->hide();
        m_closeButton->show();
    } else {
        m_timeLabel->show();
        m_closeButton->hide();
    }

    update();
}

void ItemWidget::onRefreshTime()
{
    m_timeLabel->setText(CreateTimeString(m_createTime));

    m_refreshTimer->stop();

    int interval;
    int minuteElapsed = m_createTime.secsTo(QDateTime::currentDateTime()) / 60;
    if (minuteElapsed < 60) {
        interval = 60 * 1000;
    } else {
        interval = 60 * 60 * 1000;
    }
    m_refreshTimer->start(interval);
}

void ItemWidget::initUI()
{
    //标题区域
    QWidget *titleWidget = new QWidget;
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 0, 10, 0);
    titleLayout->addWidget(m_nameLabel);
    titleLayout->addWidget(m_timeLabel);
    titleLayout->addWidget(m_closeButton);

    titleWidget->setFixedHeight(TitleHeight);

    QFont font = DFontSizeManager::instance()->t4();
    font.setWeight(75);
    m_nameLabel->setFont(font);
    m_nameLabel->setAlignment(Qt::AlignLeft);
    m_timeLabel->setAlignment(Qt::AlignRight);

    m_closeButton->setFlat(true);
    m_closeButton->setIconSize(QSize(TitleHeight / 2, TitleHeight / 2));
    m_closeButton->setFixedSize(TitleHeight, TitleHeight);
    m_closeButton->setVisible(false);

    QPalette pe = m_closeButton->palette();
    pe.setColor(QPalette::Button, pe.color(QPalette::Base));
    m_closeButton->setPalette(pe);

    m_refreshTimer->setInterval(60 * 1000);

    //布局
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->addWidget(titleWidget, 0, Qt::AlignTop);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(ContentMargin, 0, ContentMargin, 0);
    layout->addWidget(m_contentLabel, 0, Qt::AlignCenter);
    m_layout->addLayout(layout, 0);
    m_layout->addWidget(m_statusLabel, 0, Qt::AlignBottom);

    m_contentLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setFixedHeight(StatusBarHeight);
    m_statusLabel->setAlignment(Qt::AlignCenter);

    setHoverAlpha(160);
    setUnHoverAlpha(80);
    setRadius(8);

    setFocusPolicy(Qt::TabFocus);
}

void ItemWidget::initStyle(QPointer<ItemData> data)
{
    setClipType(data->title());
    setCreateTime(data->time());

    switch (data->type()) {
    case ItemData::Text: {
        setText(data->text(), data->subTitle());
    }
    break;
    case ItemData::Image: {
        m_contentLabel->setAlignment(Qt::AlignCenter);
        m_pixmap = data->pixmap();
        setPixmap(data->pixmap());
    }
    break;
    case ItemData::File: {
        if (data->urls().size() == 0)
            return;

        QString first = data->urls().first().toString();
        if (data->urls().size() == 1) {
            QFileInfo info(first);
            if (first.startsWith("file://")) {
                first.replace("file://", "");
            }
            //单个文件是图片时显示缩略图
            if (QImageReader::supportedImageFormats().contains(info.suffix().toLatin1())) {
                m_pixmap = QPixmap(first);
                setPixmap(QPixmap(first));
            } else {
                setFilePixmap(GetFileIcon(first));
            }

            m_statusLabel->setText(info.fileName());
        } else if (data->urls().size() > 1) {
            QFileInfo info(first);
            m_statusLabel->setText(tr("%1(%2 files...)").arg(info.fileName()).arg(data->urls().size()));

            int iconNum = MIN(3, data->urls().size());
            QList<QPixmap> pixmapList;
            for (int i = 0; i < iconNum; ++i) {
                QString filePath = data->urls()[i].toString();
                if (filePath.startsWith("file://")) {
                    filePath.replace("file://", "");
                }
                pixmapList.push_back(GetFileIcon(filePath));
            }
            setFilePixmaps(pixmapList);
        }
    }
    break;
    }
}

QString ItemWidget::CreateTimeString(const QDateTime &time)
{
    QString text;

    QDateTime t = QDateTime::currentDateTime();

    if (time.secsTo(t) < 60) {//60秒以内
        text = tr("Now");
    } else if (time.secsTo(t) >= 60 && time.secsTo(t) < 2 * 60) { //一分钟
        text = tr("1 minute ago");
    } else if (time.secsTo(t) >= 2 * 60 && time.secsTo(t) < 60 * 60) { //一小时内
        text = tr("%1 minutes ago").arg(time.secsTo(t) / 60);
    } else if (time.secsTo(t) >= 60 * 60 && time.secsTo(t) < 2 * 60 * 60) {//两小时内
        text = tr("1 hour ago");
    } else if (time.secsTo(t) >= 2 * 60 * 60 && time.daysTo(t) < 1) { //今天凌晨0点以后
        text = tr("%1 hours ago").arg(time.secsTo(t) / 60 / 60);
    } else if (time.daysTo(t) >= 1 && time.daysTo(t) < 2) { //今天凌晨0点以前的
        text = tr("Yesterday") + time.toString(" hh:mm");
    } else if (time.daysTo(t) >= 2 && time.daysTo(t) < 7) { //昨天凌晨0点以前的
        text = time.toString("ddd hh:mm");
    } else if (time.daysTo(t) >= 7) { //一周前0点以前的
        text = time.toString("yyyy/MM/dd");
    }

    return text;
}

void ItemWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPalette pe = this->palette();
    QColor c = pe.color(QPalette::Base);

    QPen borderPen;
    borderPen.setColor(Qt::transparent);
    painter.setPen(borderPen);

    //裁剪绘制区域
    QPainterPath path;
    path.addRoundedRect(rect(), m_radius, m_radius);
    painter.setClipPath(path);

    //绘制标题区域
    QColor brushColor(c);
    brushColor.setAlpha(80);
    painter.setBrush(brushColor);
    painter.drawRect(QRect(0, 0, width(), TitleHeight));

    //绘制背景
    brushColor.setAlpha(m_havor ? m_hoverAlpha : m_unHoverAlpha);
    painter.setBrush(brushColor);
    painter.drawRoundRect(rect(), m_radius, m_radius);

    return QWidget::paintEvent(event);
}

void ItemWidget::mousePressEvent(QMouseEvent *event)
{
    m_data->popTop();

    return DWidget::mousePressEvent(event);
}

void ItemWidget::enterEvent(QEvent *event)
{
    Q_EMIT hoverStateChanged(true);

    return DWidget::enterEvent(event);
}

void ItemWidget::leaveEvent(QEvent *event)
{
    Q_EMIT hoverStateChanged(false);

    return DWidget::leaveEvent(event);
}

void ItemWidget::focusInEvent(QFocusEvent *event)
{
    QEvent e(QEvent::Enter);
    qApp->sendEvent(this, &e);

    return DWidget::focusInEvent(event);
}

void ItemWidget::focusOutEvent(QFocusEvent *event)
{
    QEvent e(QEvent::Leave);
    qApp->sendEvent(this, &e);

    return DWidget::focusOutEvent(event);
}
