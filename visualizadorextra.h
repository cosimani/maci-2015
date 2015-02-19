#ifndef VISUALIZADOREXTRA_H
#define VISUALIZADOREXTRA_H

#include <QImage>
#include <QWidget>

namespace Ui {
class VisualizadorExtra;
}

class VisualizadorExtra : public QWidget
{
    Q_OBJECT

public:
    explicit VisualizadorExtra(QWidget *parent = 0);
    ~VisualizadorExtra();

    void setHistograma(QImage im, int posicionUmbral);

private:
    Ui::VisualizadorExtra *ui;
    void paintEvent(QPaintEvent *);
    unsigned int cantidadVisualizaciones;

    QImage im;
    int posicionUmbral;
};

#endif // VISUALIZADOREXTRA_H
