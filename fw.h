#ifndef FW_H
#define FW_H

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "population_and_order.h"

class FW : public QWidget
{
    Q_OBJECT

public:
    explicit FW(QWidget *parent = nullptr);

private slots:
    void saveFile();
    void loadFile();

private:
    QFileSystemModel *model;
    QTreeView *treeView;
};

#endif // FW_H
