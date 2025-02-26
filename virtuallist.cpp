#include "virtuallist.h"
#include <QHBoxLayout>
#include <QWheelEvent>

//设置item固定高度40
const static int nStdItemHeight = 40;

VirtualList::VirtualList(QWidget *parent) : QWidget(parent),
    m_nShowCount(0),
    m_nCurrentPos(0),
    m_nCurrentIndex(-1),
    m_nSelectedIndex(-1)
{
    QHBoxLayout* pLayout = new QHBoxLayout(this);
    m_pListView = new QListView(this);
    m_pScrollBar = new QScrollBar(this);
    m_pStdModel = new QStandardItemModel(m_pListView);
    m_pListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pListView->setResizeMode(QListView::ResizeMode::Adjust);
    m_pListView->setEditTriggers(QListView::NoEditTriggers);
    m_pListView->setModel(m_pStdModel);
    m_dataList.reserve(12000);
    pLayout->addWidget(m_pListView);
    pLayout->addWidget(m_pScrollBar);
    m_pListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pScrollBar->setSingleStep(nStdItemHeight);
    m_pScrollBar->setPageStep(nStdItemHeight);
    m_pScrollBar->hide();

    connect(m_pScrollBar,&QScrollBar::valueChanged,this,&VirtualList::OnScrollBarValueChanged);
    connect(m_pListView,&QListView::clicked,this,[&]{
        m_nSelectedIndex = m_nCurrentIndex + m_pListView->currentIndex().row();
    });
    installEventFilter(this);
    m_pListView->installEventFilter(this);
    m_pScrollBar->installEventFilter(this);
}

VirtualList::~VirtualList()
{

}

void VirtualList::Append(const QString &data)
{
    m_dataList.push_back(data);

    //数据追加后 更新索引
    if(m_nCurrentIndex < 0)
    {
        m_nCurrentIndex = 0;
    }

    //处理下滚动条
    HandleSrollBar();

    //如何当前数据的数量小于显示数量 那么刷新一次到界面
    if(m_dataList.size() - 1 < m_nShowCount)
    {
        RefreshData();
    }

}

void VirtualList::Append(const QList<QString> &dataList)
{
    m_dataList.append(dataList);

    //数据追加后 更新索引
    if(m_nCurrentIndex < 0)
    {
        m_nCurrentIndex = 0;
    }

    //处理下滚动条
    HandleSrollBar();

    //如果当前数据的数量小于显示数量 那么要刷新一次界面
    if(m_dataList.size()-dataList.size() < m_nShowCount)
    {
        RefreshData();
    }

}

void VirtualList::Clear()
{
    m_dataList.clear();
    m_nCurrentPos = 0;
    m_nCurrentIndex = -1;
    m_nSelectedIndex = -1;
    for (int i=m_pStdModel->rowCount()-1; i>=0; i--)
    {
        QStandardItem* item = m_pStdModel->item(i,0);
        delete item;
        item = nullptr;
        m_pStdModel->removeRow(i);
    }

}

QListView *VirtualList::GetListView() const
{
    return m_pListView;

}

void VirtualList::RefreshData()
{
    //界面最终展示多少个
    int showCount = m_nShowCount;

    //最后数据小于应该显示的数量时，重新计算下数量
    if(showCount + m_nCurrentIndex >= m_dataList.size())
    {
        showCount = m_dataList.size() - m_nCurrentIndex;
    }
    if(showCount > 0)
    {
        int diff = std::abs(showCount - m_pStdModel->rowCount());
        if(showCount > m_pStdModel->rowCount())//展示的数量大于当前的 要添加
        {
            for(int i=0; i<diff; i++)
            {
                QStandardItem* pItem = new QStandardItem;
                pItem->setSizeHint(QSize(0,nStdItemHeight));
                m_pStdModel->appendRow(pItem);
            }
        }
        else if(showCount < m_pStdModel->rowCount())//展示的数量小于当前的 要删除
        {
            for (int i=m_pStdModel->rowCount()-1; i>=showCount; i--) {
                QStandardItem* item = m_pStdModel->item(i,0);
                delete item;
                item = nullptr;
                m_pStdModel->removeRow(i);
            }
        }
        else
        {}

        for(int i=0; i<showCount; i++)
        {
            QStandardItem* pItem = m_pStdModel->item(i);
            pItem->setSizeHint(QSize(CalcTextWidth(pItem->font(),m_dataList[m_nCurrentIndex+i]),
                               nStdItemHeight));
            pItem->setText(m_dataList[m_nCurrentIndex+i]);
        }
    }

}

int VirtualList::RowCount() const
{
    return m_dataList.size();

}

int VirtualList::ColumnCount() const
{
    return 1;
}

int VirtualList::GetSelectedIndex() const
{
    return m_nSelectedIndex;
}

QString VirtualList::GetData(int index) const
{
    if(index<0 || index >= m_dataList.size())
    {
        return {};
    }
    return m_dataList[index];
}

QList<QString> VirtualList::GetAllData() const
{
    return m_dataList;
}

void VirtualList::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    //获取重置的窗口大小
    QSize listSize = m_pListView->size();
    m_nShowCount = listSize.height() / nStdItemHeight;
    if(m_nShowCount > 0 && m_dataList.size() > 0)
    {
        HandleSrollBar();

        //填充数据
        RefreshData();
    }

}

bool VirtualList::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::Wheel)
    {
        if(obj == m_pScrollBar || obj == m_pListView)
        {
            QWheelEvent* we = static_cast<QWheelEvent*>(event);
            if(we->angleDelta().y() > 0)//向上滚
            {
                //每滚动一次变一格
                m_pScrollBar->setValue(m_pScrollBar->value()-nStdItemHeight);
            }
            else//向下滚
            {
                m_pScrollBar->setValue(m_pScrollBar->value() + nStdItemHeight);
            }
            return true;
        }
    }
    return QWidget::eventFilter(obj,event);
}

void VirtualList::OnScrollBarValueChanged(int value)
{
    //查看最新的数据索引（最新索引是指即将要展示的首个数据索引）
    int newIndex = std::ceil(value / (nStdItemHeight*1.0f));

    //最新的索引超出数组大小
    if(newIndex >= m_dataList.size())
    {
        //最新的和当前的索引差值小于展示的数量时，直接退出
        if(std::abs(newIndex-m_nCurrentIndex) <= m_nShowCount)
            return;
        else
            --newIndex;
    }
    //数据索引刷新
    m_nCurrentIndex = newIndex;

    //取消选中状态
    m_pListView->clearSelection();

    //看看刚才的选中索引是否在范围内
    if(m_nSelectedIndex >= 0 && m_nSelectedIndex >= newIndex && m_nSelectedIndex<= newIndex + m_nShowCount)
    {
        int indexDiff = m_nSelectedIndex - m_nCurrentIndex;
        m_pListView->setCurrentIndex(m_pStdModel->index(indexDiff,0));
    }

    //做下越界防护
    if(m_nCurrentIndex >= m_dataList.size())
    {
        m_nCurrentIndex = m_dataList.size() - 1;
    }
    if(m_nCurrentIndex < 0)
    {
        m_nCurrentIndex = 0;
    }

    //重置页面数据
    RefreshData();
}

void VirtualList::HandleSrollBar()
{
    if(m_dataList.size() > m_nShowCount)
    {
        //设置大小
        m_pScrollBar->setRange(0,nStdItemHeight*m_dataList.size());
        if(m_nShowCount < m_dataList.size())//显示数量小于总数量
        {
            m_pScrollBar->setVisible(true);//滚动条显示出来
        }
    }
}

int VirtualList::CalcTextWidth(const QFont &font, const QString &text)
{
    QFontMetrics fm(font);
    return fm.horizontalAdvance(text);
}
