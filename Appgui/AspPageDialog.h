#ifndef ASPPAGEDIALOG_H
#define ASPPAGEDIALOG_H

#include <set>
#include <QDialog>
#include <QTableWidget>
#include <QTreeWidgetItem>
#include <QPainter>
#include <QStyledItemDelegate>

namespace Ui {
class AspPageDialog;
}

namespace napatahti {

class AspPage;
class AspDialog;
class ConfigDialog;


class AspPageDialog : public QDialog
{
    Q_OBJECT

public :
    explicit AspPageDialog(AspPage& aspPage, QWidget* root=nullptr);
    ~AspPageDialog();

    static auto validateText(const QString& text, const QTreeWidgetItem* root) -> bool;
    static auto validateText(const QString& text, double min=0, double max=10) -> double;

signals :
    void aspPageChanged(int kMask, int cMask);
    void aspPageNeedSync();

public slots :
    void reloadPage();

private slots :
    void onClickTree(const QTreeWidgetItem* item);
    void onTreeContext(const QPoint& pos);

    void onCellDoubleClick(int row, int column);
    void onCellChange(int row, int column);
    void onTableContext(QPoint pos);

    void onExecNewPage();
    void onExecNewSamp();
    void onExecRename4T();
    void onExecDelete4T();
    void onExecSavePageAs();

    void onSavePageAs(const QString& text);
    void onCreatePage(const QString& text);
    void onRenamePage(const QString& text);
    void onCreateSamp(const QString& text);
    void onRenameSamp(const QString& text);

    void onClickApply();
    void onClickDelete();
    void onClickCut();
    void onClickCopy();
    void onClickPaste();
    void onClickStar();

    void onDestroy() const;

private :
    Ui::AspPageDialog* ui;
    AspDialog* aspDialog_;

    AspPage& aspPage_;

    QTreeWidgetItem* pageItem_;
    QTreeWidgetItem* sampItem_;

    QList<bool>  isDump_ {false, false};
    QList<QFont> aspFont_;
    QList<QVariantList> clipboard_;

    static QString sampPath_;
    static std::array<QBrush, 2> brushED_;

    static const QRegularExpression pageReg_;

private :
    auto keyPressEvent(QKeyEvent* event) -> void;

    auto reloadTree() -> void;
    auto setWinTitle(const QString& title="") -> void;
    auto selectedRows() const -> std::set<int>;

    auto savePage() -> const QString&;
    auto loadPage(const QString& title="") -> void;
    auto dumpSamp() const -> void;
    auto loadSamp() const -> QVariantMap;

    auto execAspDialog(bool mode, double asp=-1) -> void;

    friend auto staticInit(QApplication& app) -> void;
    friend ConfigDialog;
};


class LineDelegate : public QStyledItemDelegate
{
public :
    LineDelegate(const std::map<double, QPen>& aspPen, const QTableWidget* table)
        : aspPen_ (aspPen), table_ (table)
    {}

protected :
    auto paint(QPainter* painter, const QStyleOptionViewItem& opt, const QModelIndex& ind) const -> void;

private :
    const std::map<double, QPen>& aspPen_;
    const QTableWidget* table_;

    static QPen markPen_;

    friend ConfigDialog;
};

} // namespace napatahti

#endif // ASPPAGEDIALOG_H
