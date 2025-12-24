#include "edgepropertydialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QDialogButtonBox>

EdgePropertyDialog::EdgePropertyDialog(QWidget *parent) : QDialog(parent) {
    auto *layout = new QVBoxLayout(this);

    // 长度
    auto *lenLayout = new QHBoxLayout;
    lenLayout->addWidget(new QLabel("长度:"));
    lengthEdit = new QLineEdit;
    lenLayout->addWidget(lengthEdit);
    layout->addLayout(lenLayout);

    // 是否高速
    highwayCheck = new QCheckBox("是否高速");
    layout->addWidget(highwayCheck);

    // 普通价格
    auto *priceLayout = new QHBoxLayout;
    priceLayout->addWidget(new QLabel("价格:"));
    priceEdit = new QLineEdit;
    priceLayout->addWidget(priceEdit);
    layout->addLayout(priceLayout);

    // 高速价格
    auto *highwayPriceLayout = new QHBoxLayout;
    highwayPriceLayout->addWidget(new QLabel("高速价格:"));
    highwayPriceEdit = new QLineEdit;
    highwayPriceLayout->addWidget(highwayPriceEdit);
    layout->addLayout(highwayPriceLayout);

    // 按钮
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

double EdgePropertyDialog::length() const {
    return lengthEdit->text().toDouble();
}

bool EdgePropertyDialog::isHighway() const {
    return highwayCheck->isChecked();
}

double EdgePropertyDialog::price() const {
    return priceEdit->text().toDouble();
}

double EdgePropertyDialog::highwayPrice() const {
    return highwayPriceEdit->text().toDouble();
}
