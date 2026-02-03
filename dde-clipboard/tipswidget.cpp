// SPDX-FileCopyrightText: 2018 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tipswidget.h"
#include "constants.h"

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>

#include <DFontSizeManager>
#include <DIconButton>
#include <DPalette>

TipsWidget::TipsWidget(QWidget *parent)
    : DWidget(parent)
    , m_tipsCloseButton(nullptr)
{
    initUI();
}

void TipsWidget::initUI()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    
    QLabel *tipsLabel = new QLabel(tr("Double-click the content area to copy"));
    DFontSizeManager::instance()->bind(tipsLabel, DFontSizeManager::T7);
    
    // Use DPalette::TextTips for tips text color
    DPalette palette = tipsLabel->palette();
    palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
    tipsLabel->setPalette(palette);

    m_tipsCloseButton = new DIconButton(this);
    m_tipsCloseButton->setIcon(QIcon::fromTheme("window-close"));
    m_tipsCloseButton->setFlat(true);
    m_tipsCloseButton->setFocusPolicy(Qt::NoFocus);
    m_tipsCloseButton->setFixedSize(QSize(16, 16));
    m_tipsCloseButton->setIconSize(QSize(10, 10));
    m_tipsCloseButton->setVisible(false);
    connect(m_tipsCloseButton, &DIconButton::clicked, this, [this]() {
        emit closeTips();
    });

    layout->addWidget(tipsLabel, 0, Qt::AlignVCenter);
    layout->addStretch();
    layout->addWidget(m_tipsCloseButton, 0, Qt::AlignVCenter);
}

void TipsWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPalette pe = this->palette();
    QColor c = pe.color(QPalette::Base);

    painter.setPen(Qt::NoPen);
    QColor brushColor(c);
    brushColor.setAlpha(60);
    painter.setBrush(brushColor);

    QPainterPath path;
    path.addRoundedRect(rect(), 6, 6);
    painter.drawPath(path);

    return DWidget::paintEvent(event);
}

void TipsWidget::enterEvent(QEnterEvent *event)
{
    m_tipsCloseButton->setVisible(true);

    return DWidget::enterEvent(event);
}

void TipsWidget::leaveEvent(QEvent *event)
{
    m_tipsCloseButton->setVisible(false);

    return DWidget::leaveEvent(event);
}
