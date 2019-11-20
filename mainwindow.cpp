#include "mainwindow.h"
#include "constants.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

MainWindow::MainWindow(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_displayInter(new DBusDisplay(this))
    , m_listview(new QListView(this))
    , m_model(new ClipboardModel(m_listview))
    , m_itemDelegate(new ItemDelegate)
    , m_dockInter(new DBusDock)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool  | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);

    initUI();
    initConnect();

    geometryChanged();

    installEventFilter(this);
}

MainWindow::~MainWindow()
{

}

void MainWindow::Toggle()
{
    setVisible(windowState() == Qt::WindowMinimized || !isVisible());

    if (isVisible()) {
        //显示后，在桌面Super+D快捷键，再点击桌面空白处，此时无法show出，需active
        activateWindow();
    }
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

    connect(m_model, &ClipboardModel::dataAdded, this, [ = ] {
        m_clearButton->setVisible(m_model->data().size() != 0);
    });

    connect(m_model, &ClipboardModel::dataAllCleared, this, [ = ] {
        m_clearButton->setVisible(m_model->data().size() != 0);
    });

    connect(m_model, &ClipboardModel::dataRemoved, this, [ & ] {
        m_clearButton->setVisible(m_model->data().size() != 0);
    });

    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &MainWindow::geometryChanged, Qt::UniqueConnection);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (obj)
        qDebug() << e->type();
    return false;
}

void MainWindow::geometryChanged()
{
    // 屏幕尺寸
    QRect rect = m_displayInter->primaryRawRect();
    rect.setWidth(WindowWidth + WindowMargin * 2);

    QRect dockRect = m_dockInter->frontendRect();

    switch (m_dockInter->position()) {
    case DOCK_TOP:
        rect.moveTop(dockRect.height());
    case DOCK_BOTTOM:
        rect.setHeight(rect.height() - dockRect.height());
        break;
    case DOCK_LEFT:
        rect.moveLeft(dockRect.width());
        break;
    default:;
    }

    rect = rect.marginsRemoved(QMargins(WindowMargin, WindowMargin, WindowMargin, WindowMargin));
    setGeometry(rect);
    setFixedSize(rect.size());
}

