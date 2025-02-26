#ifndef VIRTUALLIST_H
#define VIRTUALLIST_H

#include <QObject>
#include <QWidget>
#include <QListView>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QEvent>

class VirtualList : public QWidget
{
    Q_OBJECT
public:
    explicit VirtualList(QWidget *parent = nullptr);
    ~VirtualList();

public:
    //追加单个数据
    void Append(const QString& data);

    //批量追加
    void Append(const QList<QString>& dataList);

    void Clear();

    QListView *GetListView() const;

    void RefreshData();

    int RowCount() const;

    int ColumnCount() const;

    int GetSelectedIndex() const;

    //获取指定行的数据
    //index 行号  索引从0开始
    QString GetData(int index) const;

    QList<QString> GetAllData() const;

protected:
    void resizeEvent(QResizeEvent *event) override;

    bool eventFilter(QObject* obj, QEvent* event) override;

protected:
    void OnScrollBarValueChanged(int value);

    //处理滚动条 是否显示、长度区间
    void HandleSrollBar();

private:
    int CalcTextWidth(const QFont& font,const QString& text);

private:
    QListView* m_pListView;
    QScrollBar* m_pScrollBar;
    QList<QString> m_dataList;
    QStandardItemModel* m_pStdModel;
    int m_nShowCount;//显示数量
    int m_nCurrentPos;//滚动条当前位置
    int m_nCurrentIndex;//当前数据索引
    int m_nSelectedIndex;//选中的索引

};

#endif // VIRTUALLIST_H
