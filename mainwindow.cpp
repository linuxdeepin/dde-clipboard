#include "mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

// 边距
#define WINDOW_MARGIN 10

// 隐藏后的剩余空间
#define WINDOW_LEAVE 3

#define WINDOW_WIDTH 300

MainWindow::MainWindow(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_displayInter(new DBusDisplay(this))
    , m_listview(new QListView(this))
    , m_model(new ClipboardModel(m_listview))
    , m_itemDelegate(new ItemDelegate)
    , m_wmHelper(DWindowManagerHelper::instance())
    , m_showAni(new QVariantAnimation(this))
    , m_hideAni(new QVariantAnimation(this))
{
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);

    m_showAni->setEasingCurve(QEasingCurve::InOutCubic);
    m_hideAni->setEasingCurve(QEasingCurve::InOutCubic);

    m_showAni->setDuration(m_wmHelper->hasComposite() ? 300 : 1);
    m_hideAni->setDuration(m_wmHelper->hasComposite() ? 300 : 1);

    initUI();
    initConnect();

    geometryChanged();
}

MainWindow::~MainWindow()
{

}

void MainWindow::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // title
    QWidget *titleWidget = new QWidget;
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setContentsMargins(20, 0, 10, 0);

    QLabel *titleLabel = new QLabel(tr("Clipboard"));
    titleLabel->setFont(DFontSizeManager::instance()->t3());

    QPushButton *titleButton = new QPushButton(tr("Clear all"));
    connect(titleButton, &QPushButton::clicked, m_model, &ClipboardModel::clear);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        QPalette pa = titleLabel->palette();
        pa.setBrush(QPalette::WindowText, pa.brightText());
        titleLabel->setPalette(pa);

        pa = titleButton->palette();
        pa.setBrush(QPalette::ButtonText, pa.brightText());
        titleButton->setPalette(pa);
    });

    titleLayout->addWidget(titleLabel, 0, Qt::AlignLeft);
    titleLayout->addWidget(titleButton, 1, Qt::AlignRight);
    titleWidget->setFixedSize(WINDOW_WIDTH, 56);

    // list
    m_listview->setModel(m_model);
    m_listview->setItemDelegate(m_itemDelegate);
    m_listview->setAutoFillBackground(false);
    m_listview->viewport()->setAutoFillBackground(false);
    m_listview->setFrameStyle(QFrame::NoFrame);
    m_listview->setUpdatesEnabled(true);
    m_listview->setSelectionMode(QListView::NoSelection);

    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(m_listview);
    setLayout(mainLayout);
}

void MainWindow::initConnect()
{
    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &MainWindow::geometryChanged, Qt::QueuedConnection);

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, [this] {
        m_showAni->setDuration(m_wmHelper->hasComposite() ? 300 : 1);
        m_hideAni->setDuration(m_wmHelper->hasComposite() ? 300 : 1);
    });

    connect(m_showAni, &QVariantAnimation::valueChanged, [ this ](const QVariant & value) {

        if (m_showAni->state() != QPropertyAnimation::Running)
            return;

        move(value.toPoint());
    });

    connect(m_hideAni, &QVariantAnimation::valueChanged, [ this ](const QVariant & value) {

        if (m_hideAni->state() != QPropertyAnimation::Running)
            return;

        move(value.toPoint());
    });
}

void MainWindow::enterEvent(QEvent *event)
{
    if (pos().x() < 0) {
        m_showAni->setStartValue(QPoint(-width() + WINDOW_LEAVE, WINDOW_MARGIN));
        m_showAni->setEndValue(QPoint(WINDOW_MARGIN, WINDOW_MARGIN));
        m_showAni->start();
    }
    DBlurEffectWidget::enterEvent(event);
}

void MainWindow::leaveEvent(QEvent *event)
{
    if (cursor().pos().x() > WINDOW_MARGIN) {
        m_showAni->stop();
        m_hideAni->setStartValue(pos());
        m_hideAni->setEndValue(QPoint(-width() + WINDOW_LEAVE, WINDOW_MARGIN));
        m_hideAni->start();
    }

    DBlurEffectWidget::leaveEvent(event);
}

void MainWindow::geometryChanged()
{
    // 屏幕尺寸
    QRect rect = m_displayInter->primaryRawRect();
    rect.setWidth(WINDOW_WIDTH + WINDOW_MARGIN * 2);
    rect.moveLeft(-rect.width() + WINDOW_MARGIN + WINDOW_LEAVE);
    setGeometry(rect.marginsRemoved(QMargins(WINDOW_MARGIN, WINDOW_MARGIN, WINDOW_MARGIN, WINDOW_MARGIN)));
}
