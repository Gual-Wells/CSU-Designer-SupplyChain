#ifndef NODEPROPERTYDIALOG_H
#define NODEPROPERTYDIALOG_H

#include <QDialog>
#include <QVector>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class NodePropertyDialog : public QDialog {
    Q_OBJECT

public:
    explicit NodePropertyDialog(int colcount, bool isClient, QWidget *parent = nullptr);

    void setLabel(const QString &text);
    QString label() const;

    void setGoodsWants(const QVector<int> &wants);
    QVector<int> goodsWants() const;

    void setCarNum(int num);
    int carNum() const;
    void setHopeTimes(const QVector<double> &times);
    QVector<double> hopeTimes()const;


private:
    QLineEdit *labelEdit;
    QVector<QSpinBox*> goodsInputs;
    QSpinBox *carSpin;
    bool showCarSpin;
    QVector<QDoubleSpinBox*> hopeTimeInputs;


};

#endif // NODEPROPERTYDIALOG_H
