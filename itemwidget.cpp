/*
 * Copyright (C) 2018 ~ 2025 Deepin Technology Co., Ltd.
 *
 * Author:     fanpengcheng <fanpengcheng_cm@deepin.com>
 *
 * Maintainer: fanpengcheng <fanpengcheng_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
#include <QMimeDatabase>
#include <QIcon>
#include <QScopedPointer>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include "dgiofile.h"
#include "dgiofileinfo.h"

static QPixmap GetFileIcon(QString path)
{
    QFileInfo info(path);
    QIcon icon;
    QScopedPointer<DGioFile> file(DGioFile::createFromPath(info.absoluteFilePath()));
    QExplicitlySharedDataPointer<DGioFileInfo> fileinfo = file->createFileInfo();
    if (!fileinfo) {
        return icon.pixmap(FileIconWidth, FileIconWidth);
    }

    QStringList icons = fileinfo->themedIconNames();
    if (!icons.isEmpty()) return QIcon::fromTheme(icons.first()).pixmap(FileIconWidth, FileIconWidth);
    QString iconStr(fileinfo->iconString());
    if (iconStr.startsWith('/')) {
        icon = QIcon(iconStr);
    } else {
        icon = QIcon::fromTheme(iconStr);
    }

    return icon.pixmap(FileIconWidth, FileIconWidth);
}

ItemWidget::ItemWidget(QPointer<ItemData> data, QWidget *parent)
    : DWidget(parent)
    , m_data(data)
    , m_nameLabel(new DLabel(this))
    , m_timeLabel(new DLabel(this))
    , m_closeButton(new IconButton(this))
    , m_contentLabel(new PixmapLabel(this))
    , m_statusLabel(new DLabel(this))
    , m_refreshTimer(new QTimer(this))
    , m_layout(new QVBoxLayout(this))
{
    initUI();
    initData(m_data);
    initConnect();
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
        m_statusLabel->setText(QString("%1X%2px").arg(pixmap.width()).arg(pixmap.height()));
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

    titleWidget->setFixedHeight(ItemTitleHeight);

    QFont font = DFontSizeManager::instance()->t4();
    font.setWeight(75);
    m_nameLabel->setFont(font);
    m_nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_timeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    m_closeButton->setFixedSize(QSize(ItemTitleHeight, ItemTitleHeight) * 2 / 3);
    m_closeButton->setRadius(ItemTitleHeight);
    m_closeButton->setVisible(false);

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
    m_statusLabel->setFixedHeight(ItemStatusBarHeight);
    m_statusLabel->setAlignment(Qt::AlignCenter);

    setHoverAlpha(160);
    setUnHoverAlpha(80);
    setRadius(8);

    setFocusPolicy(Qt::StrongFocus);

    setMouseTracking(true);
    m_contentLabel->setMouseTracking(true);
    m_nameLabel->setMouseTracking(true);
    m_timeLabel->setMouseTracking(true);
    m_statusLabel->setMouseTracking(true);
}

void ItemWidget::initData(QPointer<ItemData> data)
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

        QUrl url = data->urls().first();
        if (data->urls().size() == 1) {
            //单个文件是图片时显示缩略图
            QImageReader imageReader(url.path());
            imageReader.setDecideFormatFromContent(true);
            if (QImageReader::supportedImageFormats().contains(imageReader.format())) {
                QImage image = imageReader.read();

                m_pixmap = QPixmap::fromImage(image);
                setPixmap(m_pixmap);
            } else {
                setFilePixmap(GetFileIcon(url.path()));
            }

            QFontMetrics metrix = m_statusLabel->fontMetrics();
            QString text = metrix.elidedText(url.fileName(), Qt::ElideMiddle, WindowWidth - 2 * ItemMargin - 10, 0);
            m_statusLabel->setText(text);

        } else if (data->urls().size() > 1) {

            QFontMetrics metrix = m_statusLabel->fontMetrics();
            QString text = metrix.elidedText(tr("%1(%2 files...)").arg(url.fileName()).arg(data->urls().size()),
                                             Qt::ElideMiddle, WindowWidth - 2 * ItemMargin - 10, 0);
            m_statusLabel->setText(text);

            int iconNum = MIN(3, data->urls().size());
            QList<QPixmap> pixmapList;
            for (int i = 0; i < iconNum; ++i) {
                QString filePath = data->urls()[i].toString();
                if (filePath.startsWith("file://")) {
                    filePath = filePath.mid(QString("file://").length());
                }
                pixmapList.push_back(GetFileIcon(filePath));
            }
            setFilePixmaps(pixmapList);
        }
    }
    break;
    default:
        break;
    }
}

void ItemWidget::initConnect()
{
    connect(m_refreshTimer, &QTimer::timeout, this, &ItemWidget::onRefreshTime);
    m_refreshTimer->start();
    connect(this, &ItemWidget::hoverStateChanged, this, &ItemWidget::onHoverStateChanged);
    connect(m_closeButton, &IconButton::clicked, [ = ] {
        m_data->remove();
    });
    connect(this, &ItemWidget::closeHasFocus, this, [&](bool has) {
        m_closeButton->setFocusState(has);
    });

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        setPixmap(m_pixmap);
    });
}

QString ItemWidget::CreateTimeString(const QDateTime &time)
{
    QString text;

    QDateTime t = QDateTime::currentDateTime();

    if (time.secsTo(t) < 60 && time.secsTo(t) >= 0) { //60秒以内
        text = tr("Now");
    } else if (time.secsTo(t) >= 60 && time.secsTo(t) < 2 * 60) { //一分钟
        text = tr("1 minute ago");
    } else if (time.secsTo(t) >= 2 * 60 && time.secsTo(t) < 60 * 60) { //多少分钟前
        text = tr("%1 minutes ago").arg(time.secsTo(t) / 60);
    } else if (time.secsTo(t) >= 60 * 60 && time.secsTo(t) < 2 * 60 * 60) {//一小时前
        text = tr("1 hour ago");
    } else if (time.secsTo(t) >= 2 * 60 * 60 && time.daysTo(t) < 1) { //多少小时前(0点以后)
        text = tr("%1 hours ago").arg(time.secsTo(t) / 60 / 60);
    } else if (time.daysTo(t) >= 1 && time.daysTo(t) < 2) { //昨天发生的
        text = tr("Yesterday") + time.toString(" hh:mm");
    } else if (time.daysTo(t) >= 2 && time.daysTo(t) < 7) { //昨天以前，一周以内
        text = time.toString("ddd hh:mm");
    } else if (time.daysTo(t) >= 7) { //一周前以前的
        text = time.toString("yyyy/MM/dd");
    }

    return text;
}

void ItemWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_0:
        //表示切换‘焦点’，tab按键事件在delegate中已被拦截
        if (event->text() == "change focus") {
            Q_EMIT closeHasFocus(m_closeFocus = !m_closeFocus);
            return ;
        }
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        if (m_closeFocus) {
            m_data->remove();
        }
    }
    break;
    }

    return DWidget::keyPressEvent(event);
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
    painter.drawRect(QRect(0, 0, width(), ItemTitleHeight));

    //绘制背景
    brushColor.setAlpha(m_havor ? m_hoverAlpha : m_unHoverAlpha);
    painter.setBrush(brushColor);
    painter.drawRect(rect());

    return DWidget::paintEvent(event);
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
    Q_EMIT closeHasFocus(false);
    Q_EMIT hoverStateChanged(false);

    return DWidget::leaveEvent(event);
}

void ItemWidget::focusInEvent(QFocusEvent *event)
{
    Q_EMIT hoverStateChanged(true);

    return DWidget::focusInEvent(event);
}

void ItemWidget::focusOutEvent(QFocusEvent *event)
{
    m_closeFocus = false;

    Q_EMIT hoverStateChanged(false);
    Q_EMIT closeHasFocus(false);

    return DWidget::focusOutEvent(event);
}
