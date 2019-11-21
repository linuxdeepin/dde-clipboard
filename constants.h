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
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QSize>
#include <QPixmap>
#include <QPainter>
#include <QPalette>
#include <QBitmap>

#define MAX(a,b) ((a) > (b) ? (a):(b))
#define MIN(a,b) ((a) < (b) ? (a):(b))

const static int WindowWidth = 300;
const static int WindowMargin = 10;     //边距
const static int WindowLeave = 3;       //隐藏后的剩余空间
const static int TitleHeight = 36;      //Item标题栏高度
const static int StatusBarHeight = 30;  //Item状态栏高度
const static int ItemWidth = WindowWidth - 2 * WindowMargin;
const static int ItemHeight = 200;
const static int ItemMargin = 10;

const static int PixmapWidth = 180;     //图像最大显示宽度
const static int PixmapHeight = 100;    //图像最大显示高度
const static int FileIconWidth = PixmapWidth;
const static int FileIconHeight = PixmapHeight;
const static int PixmapxStep = 15;
const static int PixmapyStep = 5;
const static int ContentMargin = 21;
const static int TextContentTopOffset = 20;

static const QString DBusClipBoardService = "com.deepin.dde.Clipboard";
static const QString DBusClipBoardPath = "/com/deepin/dde/Clipboard";

namespace  Globals {
/*获取图片缩放比例*/
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

inline QPixmap pixmapScaled(const QPixmap &pixmap)
{
    qreal scale = Globals::GetScale(pixmap.size(), PixmapWidth, PixmapHeight);

    return pixmap.scaled(pixmap.size() / scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

inline QPixmap GetRoundPixmap(const QPixmap &pix, QColor borderColor)
{
    if (pix.isNull()) {
        return QPixmap();
    }

    int radius;
    int borderWidth;
    if (pix.width() > pix.height()) {
        radius = int(8 * 1.0 / PixmapWidth * pix.width());
        borderWidth = int(10 * 1.0 / PixmapWidth * pix.width());
    } else {
        radius = int(8 * 1.0 / PixmapHeight * pix.height());
        borderWidth = int(10 * 1.0 / PixmapHeight * pix.height());
    }

    QPixmap pixmap = pix;
    //绘制边框
    QPainter pixPainter(&pixmap);
    pixPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    borderColor.setAlpha(60);
    pixPainter.setPen(QPen(borderColor, borderWidth));
    pixPainter.setBrush(Qt::transparent);
    pixPainter.drawRoundedRect(pixmap.rect(), radius, radius);

    //设置圆角
    QSize size(pixmap.size());
    QBitmap mask(size);
    QPainter painter(&mask);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.fillRect(mask.rect(), Qt::color0);
    painter.setBrush(Qt::color1);
    painter.drawRoundedRect(mask.rect(), radius, radius);

    //遮罩
    QPixmap image = pixmap;
    image.setMask(mask);
    return image;
}
} ;
#endif // CONSTANTS_H
