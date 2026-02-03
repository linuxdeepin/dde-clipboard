// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QSize>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QBitmap>
#include <QImage>
#include <QTimer>
#include <QIcon>

#define MAX(a,b) ((a) > (b) ? (a):(b))
#define MIN(a,b) ((a) < (b) ? (a):(b))

inline constexpr int WindowWidth = 300;
inline constexpr int WindowMargin = 10;             //边距
inline constexpr int WindowTitleHeight = 56;
inline constexpr int ItemTitleHeight = 36;          //Item标题栏高度
inline constexpr int ItemStatusBarHeight = 30;      //Item状态栏高度
inline constexpr int ItemWidth = WindowWidth - 2 * WindowMargin;
inline constexpr int ItemHeight = 200;
inline constexpr int ItemMargin = 10;
inline constexpr int PixmapWidth = 180;             //图像最大显示宽度
inline constexpr int PixmapHeight = 100;            //图像最大显示高度
inline constexpr int FileIconWidth = PixmapWidth;
inline constexpr int FileIconHeight = PixmapHeight;
inline constexpr int PixmapxStep = 15;
inline constexpr int PixmapyStep = 5;
inline constexpr int ContentMargin = 21;
inline constexpr int TextContentTopMargin = 20;
inline constexpr int TextLineSpacing = 8;           //文本行间距
inline constexpr int MinFontLeading = 2;            // Minimum font leading for special fonts
inline constexpr int AnimationTime = 300;           //ms
inline constexpr int CloseButtonSize = 20;
inline constexpr int TipWidgetWidth = 280;
inline constexpr int TipWidgetHeight = 30;

static const QString DBusClipBoardService = "org.deepin.dde.Clipboard1";
static const QString DBusClipBoardPath = "/org/deepin/dde/Clipboard1";
static const QString ShowTipsWidget = "showTipsWidget"; // 显示提示控件

static const QString TextUriListLiteral QStringLiteral("text/uri-list");
static const QString TextHtmlLiteral QStringLiteral("text/html");
static const QString TextPlainLiteral QStringLiteral("text/plain");
static const QString ApplicationXColorLiteral QStringLiteral("application/x-color");
static const QString ApplicationXQtImageLiteral QStringLiteral("application/x-qt-image");

typedef struct {
    QStringList cornerIconList;
    QIcon fileIcon;
} FileIconData;

namespace  Globals {
/*!
 * \~chinese \name GetScale
 * \~chinese \brief 获取缩放的比例
 * \~chinese \param size 当前所需要的大小
 * \~chinese \param targetWidth  目标宽度
 * \~chinese \param targetHeight 目标高度
 */
static qreal GetScale(QSize size, int targetWidth, int targetHeight)
{
    qreal scale = 1.0;

    if (size.width() >= size.height() * (targetWidth * 1.0 / targetHeight)) {
        scale = size.width() * 1.0 / targetWidth;
    } else {
        scale = size.height() * 1.0 / targetHeight;
    }

    return scale;
}

/*!
 * \~chinese \name pixmapScaled
 * \~chinese \brief 图片缩放
 * \~chinese \param pixmap 源pixmap数据
 * \~chinese \return 返回一个宽度为180,高度为100的图片,并且缩放比例不变
 */
inline QPixmap pixmapScaled(const QPixmap &pixmap)
{
    if (pixmap.isNull())
        return pixmap;
    qreal scale = Globals::GetScale(pixmap.size(), PixmapWidth, PixmapHeight);
    return pixmap.scaled(pixmap.size() / scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

/*!
 * \~chinese \name GetRoundPixmap
 * \~chinese \brief 获取一张带有圆角的图片，边框颜色为系统调色板中的Base颜色（加透明度）
 * \~chinese \param pix 源pixmap数据
 * \~chinese \param borderColor 边框颜色
 * \~chinese \return 如果pix为空返回一个空的图片,如果不为空返回调整后的图片.
 */
inline QPixmap GetRoundPixmap(const QPixmap &pix, QColor borderColor)
{
    if (pix.isNull()) {
        return QPixmap();
    }

    const int radius = 10;
    const qreal borderWidth = 4.0;

    const qreal dpr = pix.devicePixelRatioF();
    const QSize logicalSize = pix.size();

    QImage target(logicalSize * dpr, QImage::Format_ARGB32_Premultiplied);
    target.setDevicePixelRatio(dpr);
    target.fill(Qt::transparent);

    QPainter painter(&target);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF rect(QPointF(0, 0), QSizeF(logicalSize));

    {
        QPainterPath outerClip;
        outerClip.addRoundedRect(rect, radius, radius);
        painter.setClipPath(outerClip, Qt::ReplaceClip);
        painter.setOpacity(190.0 / 255.0);
        painter.drawPixmap(rect, pix, pix.rect());
        painter.setOpacity(1.0);
    }

    const qreal inset = borderWidth;
    const QRectF innerRect = rect.adjusted(inset, inset, -inset, -inset);
    qreal innerRadius = radius - inset;
    if (innerRadius < 0.0)
        innerRadius = 0.0;

    QPainterPath innerClip;
    innerClip.addRoundedRect(innerRect, innerRadius, innerRadius);
    painter.setClipPath(innerClip, Qt::ReplaceClip);
    painter.drawPixmap(rect, pix, pix.rect());

    painter.end();
    return QPixmap::fromImage(target);
}
} ;
#endif // CONSTANTS_H
