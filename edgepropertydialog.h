#ifndef EDGEPROPERTYDIALOG_H
#define EDGEPROPERTYDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;

class EdgePropertyDialog : public QDialog {
    Q_OBJECT

public:
    EdgePropertyDialog(QWidget *parent = nullptr);

    double length() const;
    bool isHighway() const;
    double price() const;
    double highwayPrice() const;

private:
    QLineEdit *lengthEdit;
    QCheckBox *highwayCheck;
    QLineEdit *priceEdit;
    QLineEdit *highwayPriceEdit;
};

#endif // EDGEPROPERTYDIALOG_H
