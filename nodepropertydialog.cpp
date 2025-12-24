#include "nodepropertydialog.h"

NodePropertyDialog::NodePropertyDialog(int colcount, bool isClient, QWidget *parent)
    : QDialog(parent), showCarSpin(!isClient) {
    setWindowTitle("修改节点属性");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Label 输入
    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addWidget(new QLabel("名称：", this));
    labelEdit = new QLineEdit(this);
    labelLayout->addWidget(labelEdit);
    mainLayout->addLayout(labelLayout);

    QHBoxLayout *goods = new QHBoxLayout;
    goods->addWidget(new QLabel("货物：", this));
    goods->addWidget(new QLabel("A品", this));
    goods->addWidget(new QLabel("B品", this));
    goods->addWidget(new QLabel("C品", this));
    goods->addWidget(new QLabel("D品", this));
    goods->addWidget(new QLabel("E品", this));
    mainLayout->addLayout(goods);


    QString A="需求：";
    QString B="库存：";

    if(isClient){


    QHBoxLayout *goodsLayout = new QHBoxLayout;
    goodsLayout->addWidget(new QLabel(A, this));
    for (int i = 0; i < colcount; ++i) {
        QSpinBox *input = new QSpinBox(this);
        input->setRange(0, 1000);
        input->setValue(0);
        input->setAlignment(Qt::AlignCenter);
        input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        goodsInputs.append(input);
        goodsLayout->addWidget(input);
    }

    mainLayout->addLayout(goodsLayout);

    }

    else {

        QHBoxLayout *goodsLayout = new QHBoxLayout;
        goodsLayout->addWidget(new QLabel(B, this));
        for (int i = 0; i < colcount; ++i) {
            QSpinBox *input = new QSpinBox(this);
            input->setRange(0, 1000);
            input->setValue(0);
            input->setAlignment(Qt::AlignCenter);
            input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            goodsInputs.append(input);
            goodsLayout->addWidget(input);
        }
        mainLayout->addLayout(goodsLayout);

    }
    if (isClient) {
        QHBoxLayout *hopeTimeLayout = new QHBoxLayout;
        hopeTimeLayout->addWidget(new QLabel("期望：", this));
        for (int i = 0; i < colcount; ++i) {
            QDoubleSpinBox *input = new QDoubleSpinBox(this);
            input->setDecimals(2);
            input->setRange(0.0, 100000.0);
            input->setSingleStep(0.1);
            input->setValue(0.0);
            input->setAlignment(Qt::AlignCenter);
            input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            hopeTimeInputs.append(input);
            hopeTimeLayout->addWidget(input);
        }
        mainLayout->addLayout(hopeTimeLayout);
    }






    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *okBtn = new QPushButton("确定", this);
    QPushButton *cancelBtn = new QPushButton("取消", this);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);
}

void NodePropertyDialog::setLabel(const QString &text) {
    labelEdit->setText(text);
}

QString NodePropertyDialog::label() const {
    return labelEdit->text();
}

void NodePropertyDialog::setGoodsWants(const QVector<int> &wants) {
    int count = qMin(goodsInputs.size(), wants.size());
    for (int i = 0; i < count; ++i) {
        goodsInputs[i]->setValue(wants[i]);
    }
}

QVector<int> NodePropertyDialog::goodsWants() const {
    QVector<int> result;
    for (QSpinBox *input : goodsInputs) {
        result.append(input->value());
    }
    return result;
}

void NodePropertyDialog::setCarNum(int num) {
    if (carSpin) carSpin->setValue(num);
}

int NodePropertyDialog::carNum() const {
    return carSpin ? carSpin->value() : 0;
}

void NodePropertyDialog::setHopeTimes(const QVector<double> &times) {
    int count = qMin(hopeTimeInputs.size(), times.size());
    for (int i = 0; i < count; ++i) {
        hopeTimeInputs[i]->setValue(times[i]);
    }
}

QVector<double> NodePropertyDialog::hopeTimes() const {
    QVector<double> result;
    for (QDoubleSpinBox *input : hopeTimeInputs) {
        result.append(input->value());
    }
    return result;
}


