#include "visualizadorextra.h"
#include "ui_visualizadorextra.h"
#include <QPainter>
#include <QDebug>
#include <QPen>

VisualizadorExtra::VisualizadorExtra(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VisualizadorExtra)
{
    ui->setupUi(this);

    cantidadVisualizaciones = 0;
    posicionUmbral = 0;

    this->resize(256, 520);
}

VisualizadorExtra::~VisualizadorExtra()
{
    delete ui;
}

void VisualizadorExtra::paintEvent(QPaintEvent *)  {
    QPainter painter(this);

//    qDebug() << im.isNull() << "Ancho" << im.width() << "Alto" << im.height();

    QPen pen(Qt::green, 4, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);

    painter.drawImage(0, 0, im);
    painter.drawLine(posicionUmbral, im.height()-2, posicionUmbral, im.height()+15);


}

void VisualizadorExtra::setHistograma(QImage im, int posicionUmbral)  {
    this->posicionUmbral = posicionUmbral;
    this->im = im;
    this->repaint();
}

