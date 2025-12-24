#ifndef CHOOSEDIALOG_H
#define CHOOSEDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QPushButton>

class ChooseDialog : public QDialog {
    Q_OBJECT
public:
    ChooseDialog(QWidget* parent = nullptr);
    double getChooseValue() const { return chooseValue; }

private:
    double chooseValue;
    QDoubleSpinBox* chooseSpinBox;
};

#endif // CHOOSEDIALOG_H
