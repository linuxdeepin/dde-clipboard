// SPDX-FileCopyrightText: 2018 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIPSWIDGET_H
#define TIPSWIDGET_H

#include <DWidget>
#include <DLabel>
#include <DGuiApplicationHelper>
#include <DIconButton>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

/*!
 * \~chinese \class TipsWidget
 * \~chinese \brief 剪切板双击提示控件,继承于DWidget
 */
class TipsWidget : public DWidget
{
    Q_OBJECT
public:
    explicit TipsWidget(QWidget *parent = nullptr);

private:
    void initUI();
    void setLabelPalette(QLabel *label, DGuiApplicationHelper::ColorType themeType, double alphaF);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

Q_SIGNALS:
    void closeTips();

private:
    DIconButton *m_tipsCloseButton;
};

#endif // TIPSWIDGET_H
