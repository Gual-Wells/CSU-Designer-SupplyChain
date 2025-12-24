#include "choosedialog.h"

extern double choose;

ChooseDialog::ChooseDialog(QWidget* parent) : QDialog(parent), chooseValue(choose) {
    setWindowTitle("设置 Choose 值");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    chooseSpinBox = new QDoubleSpinBox(this);
    chooseSpinBox->setRange(0.01, 0.99);
    chooseSpinBox->setSingleStep(0.05);
    chooseSpinBox->setDecimals(2);
    chooseSpinBox->setValue(chooseValue);
    chooseSpinBox->setKeyboardTracking(true);
    mainLayout->addWidget(chooseSpinBox);

    QHBoxLayout* confirmLayout = new QHBoxLayout();
    QPushButton* confirmButton = new QPushButton("确认", this);
    QPushButton* cancelButton = new QPushButton("取消", this);
    confirmLayout->addWidget(confirmButton);
    confirmLayout->addWidget(cancelButton);
    mainLayout->addLayout(confirmLayout);

    connect(confirmButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    connect(chooseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value) {
        if (qFuzzyCompare(value, 0.01) || qFuzzyCompare(value, 0.99)) {
            chooseSpinBox->setSingleStep(0.04);
        } else {
            chooseSpinBox->setSingleStep(0.05);
        }
        chooseValue = value;
    });
}
