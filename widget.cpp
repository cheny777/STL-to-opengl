#include "widget.h"
#include "ui_widget.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    m_psgl = new sopengl(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_psgl);
    layout->addWidget(ui->pushButton);
    layout->addWidget(ui->comboBox);

    ui->comboBox->addItem("面");
    ui->comboBox->addItem("线");
    ui->comboBox->addItem("点");

}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    QString strname = QFileDialog::getOpenFileName(NULL,"open","./","tr(*.STL *.stl)");

    m_psgl->setstl(strname);


}

void Widget::on_comboBox_currentIndexChanged(int index)
{
    m_psgl->settypes(index);
}
