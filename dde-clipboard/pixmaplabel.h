// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H
#include "itemdata.h"

#include <QPointer>
#include <DLabel>

#include <QTextOption>

DWIDGET_USE_NAMESPACE

class QTextLayout;
/*!
 * \~chinese \class PixmapLabel
 * \~chinese \brief 继承于DLabel,DLabel继承于QLabel,用于显示剪切块中的文字和图标等信息
 */
class PixmapLabel : public DLabel
{
public:
    explicit PixmapLabel(QPointer<ItemData> data, QWidget *parent = nullptr);

    /*!
     * \~chinese \name text
     * \~chinese \brief 获取剪切板中的文字
     * \~chinese \return 返回剪切板中的文字
     */
    void setText(bool);

    inline const QList<QPixmap> pixmapList() { return m_pixmapList; }
    void setPixmapList(const QList<QPixmap> &list);

    virtual QSize minimumSizeHint() const override;
    virtual QSize sizeHint() const override;

private:
    static constexpr int MAX_TEXT_LINES = 4;        // 最大显示行数
    static constexpr const char* ELLIPSIS = "...";   // 省略号
    
    /*!
     * \~chinese \name getTextAreaWidth
     * \~chinese \brief 获取文本显示区域的宽度
     * \~chinese \return 返回可用于显示文本的宽度
     */
    inline int getTextAreaWidth() const { return ItemWidth - ContentMargin * 2; }
    /*!
     * \~chinese \name calculateTextLineCount
     * \~chinese \brief 根据当前字体动态计算文本行数
     * \~chinese \return 返回文本实际显示的行数（最多4行）
     */
    int calculateTextLineCount() const;
    
    /*!
     * \~chinese \name calculateTextLines
     * \~chinese \brief 根据当前字体动态计算文本分行，并返回是否有更多文本
     * \~chinese \param fm 字体度量对象
     * \~chinese \param outLines 输出的文本行列表
     * \~chinese \return 返回是否有更多文本被截断（超过4行）
     */
    bool calculateTextLines(const QFontMetrics &fm, QStringList &outLines) const;
    
    bool m_istext;

    QPointer<ItemData> m_data;
    QList<QPixmap> m_pixmapList;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void changeEvent(QEvent *event) override;
};

#endif // PIXMAPLABEL_H
