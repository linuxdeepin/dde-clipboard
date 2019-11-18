#include "mainwindow.h"
#include "constants.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

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
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);

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

void MainWindow::Show()
{
    m_showAni->setStartValue(QPoint(-width() + WindowLeave, WindowMargin));
    m_showAni->setEndValue(QPoint(WindowMargin, WindowMargin));
    m_showAni->start();
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

    m_clearButton = new QPushButton(tr("Clear all"));
    connect(m_clearButton, &QPushButton::clicked, m_model, &ClipboardModel::clear);
    m_clearButton->setFocusPolicy(Qt::NoFocus);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        QPalette pa = titleLabel->palette();
        pa.setBrush(QPalette::WindowText, pa.brightText());
        titleLabel->setPalette(pa);

        pa = m_clearButton->palette();
        pa.setBrush(QPalette::ButtonText, pa.brightText());
        m_clearButton->setPalette(pa);

        QPalette pe = this->palette();
        QColor base = pe.color(QPalette::Base);
        base.setAlpha(120);
        pe.setColor(QPalette::Base, base);
        pe.setColor(QPalette::Dark, base);
        pe.setColor(QPalette::Light, base);
        m_clearButton->setPalette(pe);
    });

    titleLayout->addWidget(titleLabel, 0, Qt::AlignLeft);
    titleLayout->addWidget(m_clearButton, 1, Qt::AlignRight);
    m_clearButton->setVisible(false);
    titleWidget->setFixedSize(WindowWidth, 56);

    // list
    m_listview->setModel(m_model);
    m_listview->setItemDelegate(m_itemDelegate);
    m_listview->setAutoFillBackground(false);
    m_listview->viewport()->setAutoFillBackground(false);
    m_listview->setFrameStyle(QFrame::NoFrame);
    m_listview->setSelectionMode(QListView::NoSelection);
    m_listview->setTabKeyNavigation(true);
    m_listview->setFocusPolicy(Qt::NoFocus);

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

    connect(m_model, &ClipboardModel::dataAdded, this, [ = ] {
        m_clearButton->setVisible(m_model->data().size() != 0);
    });

    connect(m_model, &ClipboardModel::dataAllCleared, this, [ = ] {
        m_clearButton->setVisible(m_model->data().size() != 0);
    });

    connect(m_model, &ClipboardModel::dataRemoved, this, [ & ] {
        m_clearButton->setVisible(m_model->data().size() != 0);
    });
}

void MainWindow::enterEvent(QEvent *event)
{
    if (pos().x() < 0) {
        m_showAni->setStartValue(QPoint(-width() + WindowLeave, WindowMargin));
        m_showAni->setEndValue(QPoint(WindowMargin, WindowMargin));
        m_showAni->start();
    }
    DBlurEffectWidget::enterEvent(event);
}

void MainWindow::leaveEvent(QEvent *event)
{
    if (cursor().pos().x() > WindowMargin) {
        m_showAni->stop();
        m_hideAni->setStartValue(pos());
        m_hideAni->setEndValue(QPoint(-width() + WindowLeave, WindowMargin));
        m_hideAni->start();
    }

    DBlurEffectWidget::leaveEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return;
}

void MainWindow::geometryChanged()
{
    // 屏幕尺寸
    QRect rect = m_displayInter->primaryRawRect();
    rect.setWidth(WindowWidth + WindowMargin * 2);
    rect.moveLeft(-rect.width() + WindowMargin + WindowLeave);
    rect = rect.marginsRemoved(QMargins(WindowMargin, WindowMargin, WindowMargin, WindowMargin));
    setFixedSize(rect.size());
    move(WindowMargin, WindowMargin);
}

void MainWindow::showEvent(QShowEvent *event)
{
    DBlurEffectWidget::showEvent(event);

    setFocus();
}
