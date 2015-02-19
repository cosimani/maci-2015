#include "hiloCaptura.h"
#include "util.h"
#include "escena.h"
#include <QDebug>

#include <QtCore/qmath.h>

#include "ui_ventanavalores.h"


using namespace cv;

HiloCaptura::HiloCaptura(QObject * parent) : activo(false), camaraActiva(0)
{
    this->ventana = (Ventana *)parent;

    if (Configuracion::tipoCamara.contains("webcam1", Qt::CaseInsensitive))  {
        cap = new VideoCapture(camaraActiva);
        cap->set(CV_CAP_PROP_FRAME_WIDTH, RESOLUTION_LOW_W);
        cap->set(CV_CAP_PROP_FRAME_HEIGHT, RESOLUTION_LOW_H);
    }
    else if (Configuracion::tipoCamara.contains("webcam2", Qt::CaseInsensitive))  {
        cap = new VideoCapture(camaraActiva + 1);
        cap->set(CV_CAP_PROP_FRAME_WIDTH, RESOLUTION_LOW_W);
        cap->set(CV_CAP_PROP_FRAME_HEIGHT, RESOLUTION_LOW_H);
    }
    else if (Configuracion::tipoCamara.contains("webcam3", Qt::CaseInsensitive))  {
        cap = new VideoCapture(camaraActiva + 2);
        cap->set(CV_CAP_PROP_FRAME_WIDTH, RESOLUTION_LOW_W);
        cap->set(CV_CAP_PROP_FRAME_HEIGHT, RESOLUTION_LOW_H);
    }

    this->frame.create(Size(RESOLUTION_LOW_W, RESOLUTION_LOW_H), CV_8UC3);

    ventanaValores.show();
    ventanaValores.move(0, 0);

    visualizadorExtra.show();
    visualizadorExtra.move(1100, 0);

    file.setFileName("../DientesBlancos/estadisticas.txt");

    file.open(QIODevice::WriteOnly | QIODevice::Text);

    streamFile.setDevice(&file);
    tiempo = 0;
    tiempo_old = 0;

    streamFile << "Se inicia la aplicacion:" << QDateTime::currentDateTime().toString("h:m:s ap") << "\n";


}

HiloCaptura::~HiloCaptura()  {
    streamFile << "Se cierra la aplicacion:" << QDateTime::currentDateTime().toString("h:m:s ap") << "\n";
    delete cap;
}


void HiloCaptura::slot_cambiarCamara()  {
    this->finalizarHilo();
    this->wait(500);  // Esperamos 500 milisegundos maximo para que finalice el hilo y podamos cambiar de camara

    do  {
        if (camaraActiva>=2)
            camaraActiva=-1;

        delete cap;
        cap = new VideoCapture(++camaraActiva);

        if (ventanaValores.ui->cbHighResolution->isChecked())  {
            cap->set(CV_CAP_PROP_FRAME_WIDTH, RESOLUTION_HIGH_W);
            cap->set(CV_CAP_PROP_FRAME_HEIGHT, RESOLUTION_HIGH_H);

            // Aqui seteamos el ancho y alto de la imagen de video esperada. Pero puede pasar que la camara no
            // pueda trabajar en esa resolucion. Utilizando el metodo get de VideCapture podemos obtener la resolucion
            // que finalmente se configura. Entonces utilizamos estos valores para redimensionar el Mat frame.

            double ancho = cap->get(CV_CAP_PROP_FRAME_WIDTH);
            double alto = cap->get(CV_CAP_PROP_FRAME_HEIGHT);

            this->frame.create(Size(ancho, alto), CV_8UC3);

            qDebug() << "\nget CV_CAP_PROP_FRAME_WIDTH" << cap->get(CV_CAP_PROP_FRAME_WIDTH)
                     << "get CV_CAP_PROP_FRAME_HEIGHT" << cap->get(CV_CAP_PROP_FRAME_HEIGHT);

            qDebug() << "camara=" << camaraActiva << "high - Frame rows=" << frame.rows
                     << "high - Frame cols=" << frame.cols;
        }
        else  {
            cap->set(CV_CAP_PROP_FRAME_WIDTH, RESOLUTION_LOW_W);
            cap->set(CV_CAP_PROP_FRAME_HEIGHT, RESOLUTION_LOW_H);

            // Aqui seteamos el ancho y alto de la imagen de video esperada. Pero puede pasar que la camara no
            // pueda trabajar en esa resolucion. Utilizando el metodo get de VideCapture podemos obtener la resolucion
            // que finalmente se configura. Entonces utilizamos estos valores para redimensionar el Mat frame.

            double ancho = cap->get(CV_CAP_PROP_FRAME_WIDTH);
            double alto = cap->get(CV_CAP_PROP_FRAME_HEIGHT);

            this->frame.create(Size(ancho, alto), CV_8UC3);

            qDebug() << "\nget CV_CAP_PROP_FRAME_WIDTH" << cap->get(CV_CAP_PROP_FRAME_WIDTH)
                     << "get CV_CAP_PROP_FRAME_HEIGHT" << cap->get(CV_CAP_PROP_FRAME_HEIGHT);

            qDebug() << "camara=" << camaraActiva << "low - Frame rows=" << frame.rows
                     << "high - Frame cols=" << frame.cols;

        }

//        cap->open(camaraActiva);
    }
    while(!cap->isOpened());

    this->start();
}

void HiloCaptura::run()  {

    this->activo = true;

    if (Configuracion::tipoCamara.contains("kinect", Qt::CaseInsensitive))  {
//        if (!kinect->kinectDetected)  {
//            qDebug() << "La camara del Kinect no pudo ser iniciada!!";
//            return;
//        }
    }
    else  {  // Aqui para cualquier webcam
        if( !cap->isOpened() )  {
            qDebug() << "La camara con VideoCapture no pudo ser iniciada!!";
            return;
        }
    }

    while(activo)  {

        tiempo = QDateTime::currentDateTime().toMSecsSinceEpoch();

        streamFile << tiempo - tiempo_old <<"\n";

        tiempo_old = tiempo;

//        QThread::msleep(500);

        if (Configuracion::tipoCamara.contains("kinect", Qt::CaseInsensitive))  {
//            kinect->getRGB(m_rgb);

//            Mat frame(Size(RESOLUTION_W, RESOLUTION_H), CV_8UC3, (void*)m_rgb.data());
//            this->frame = frame;
        }
        else  {  // Aqui para cualquier webcam
            cap->operator >>(frame);
        }

        vector<Rect> faces;
        faces = detectorCara.detectFacesRect(frame);

        if((faces.size() > 0) && (ventanaValores.ui->cbRostro->isChecked()))   {  // Encontramos una cara

            faces.at(0).height *= CORRECCION_BOCA_ABIERTA;  // Para bajar un poco mas la linea inferior para cuando se abre la boca

            if (ventanaValores.ui->cbLineas->isChecked())
                detectorCara.drawMultipleRect(faces, frame);  // Dibuja el Rectángulo de la cara
            Rect roiFace(faces.at(0).x, faces.at(0).y, faces.at(0).width, faces.at(0).height);
            Mat middleFace(frame, roiFace);

            this->frame = middleFace.clone(); // Trabajamos solo sobre la cara

            int anchoImagenROIFace = frame.cols;
            int altoImagenROIFace = frame.rows;

            // Rectangulos Grandes
            Rect rOjoIzquierdo(anchoImagenROIFace/10, altoImagenROIFace/CORRECCION_BOCA_ABIERTA/10,
                               anchoImagenROIFace*4/10, altoImagenROIFace/CORRECCION_BOCA_ABIERTA*5/10);            
            Rect rOjoDerecho(anchoImagenROIFace*5/10, altoImagenROIFace/CORRECCION_BOCA_ABIERTA/10,
                             anchoImagenROIFace*4/10, altoImagenROIFace/CORRECCION_BOCA_ABIERTA*5/10);
            Mat matOjoIzquierdo(frame, rOjoIzquierdo);
            Mat matOjoDerecho(frame, rOjoDerecho);

            if (ventanaValores.ui->cbLineas->isChecked())  {
                rectangle(frame, rOjoIzquierdo, Scalar( 0, 255, 0 ), 2, 8, 0);
                rectangle(frame, rOjoDerecho, Scalar( 0, 255, 0 ), 2, 8, 0);
            }

            // Rectangulos de Ojos
            vector<Rect> ojoIzquierdo = detectorCara.detectOjoIzquierdoRect(matOjoIzquierdo);
            if (ventanaValores.ui->cbLineas->isChecked())
                detectorCara.drawMultipleRect(ojoIzquierdo, matOjoIzquierdo);
            vector<Rect> ojoDerecho = detectorCara.detectOjoDerechoRect(matOjoDerecho);
            if (ventanaValores.ui->cbLineas->isChecked())
                detectorCara.drawMultipleRect(ojoDerecho, matOjoDerecho);

            // Calculamos lineas de borde
            Point centroOjoIzquierdo;
            if(ojoIzquierdo.size() > 0)  {
                centroOjoIzquierdo.x = anchoImagenROIFace/10 + ojoIzquierdo.at(0).x + ojoIzquierdo.at(0).width/4;
                centroOjoIzquierdo.y = altoImagenROIFace/10 + ojoIzquierdo.at(0).y + ojoIzquierdo.at(0).height/2;
            }



            Point centroOjoDerecho;
            if (ojoDerecho.size() > 0)  {
                centroOjoDerecho.x = anchoImagenROIFace*5/10 + ojoDerecho.at(0).x + ojoDerecho.at(0).width*3/4;
                centroOjoDerecho.y = altoImagenROIFace/10 + ojoDerecho.at(0).y +ojoDerecho.at(0).height/2;
            }

            float anguloEntreOjos = 0;
            if (ojoIzquierdo.size() > 0 && ojoDerecho.size() > 0 )
                anguloEntreOjos = angulo(centroOjoIzquierdo, centroOjoDerecho);

            bool caso = (centroOjoIzquierdo.y < centroOjoDerecho.y);
            Point centroInferiorIzquierdo = inferiorDe(centroOjoIzquierdo, anguloEntreOjos, caso);
            Point centroInferiorDerecho = inferiorDe(centroOjoDerecho, anguloEntreOjos, caso);

            if (ventanaValores.ui->cbLineas->isChecked())
                if (ojoIzquierdo.size() > 0 && ojoDerecho.size() > 0)  {  // Dibuja las lineas verticales
                    line(frame, centroOjoIzquierdo, centroInferiorIzquierdo, Scalar(255, 255, 0), 2);
                    line(frame, centroOjoDerecho, centroInferiorDerecho, Scalar(255, 255, 0), 2);
                }


            // Borde de boca Superior
            Point bocaSupIzq = puntoNuevo(centroOjoIzquierdo, anguloEntreOjos, caso,
                                          altoImagenROIFace/CORRECCION_BOCA_ABIERTA/3);  // Por que es esa division por 3 ?
            bocaSupIzq.y /= CORRECCION_LINEA_SUPERIOR_BOCA;
            Point bocaSupDer = puntoNuevo(centroOjoDerecho, anguloEntreOjos, caso,
                                          altoImagenROIFace/CORRECCION_BOCA_ABIERTA/3);
            bocaSupDer.y /= CORRECCION_LINEA_SUPERIOR_BOCA;

            // Borde de boca Inferior
            Point bocaInfIzq = puntoNuevo(centroOjoIzquierdo, anguloEntreOjos, caso, altoImagenROIFace*4/7);  // Por que 4/7 ?
            Point bocaInfDer = puntoNuevo(centroOjoDerecho, anguloEntreOjos, caso, altoImagenROIFace*4/7);

            if (ventanaValores.ui->cbLineas->isChecked())
                if (ojoIzquierdo.size() > 0 && ojoDerecho.size() > 0)  {  // Dibuja los bordes superior e inferior de la boca
                    line(frame, bocaSupIzq, bocaSupDer, Scalar(255, 255, 0), 2);
                    line(frame, bocaInfIzq, bocaInfDer, Scalar(255, 255, 0), 2);
                }

            Point roiSupIzq;
            Point roiInfDer;

            if (bocaInfIzq.x < bocaSupIzq.x)  {
                roiSupIzq.x = bocaInfIzq.x;
                roiSupIzq.y = bocaSupIzq.y;
                roiInfDer.x = bocaSupDer.x;
                roiInfDer.y = bocaInfDer.y;
            }
            else  {
                roiSupIzq.x = bocaSupIzq.x;
                roiSupIzq.y = bocaSupDer.y;
                roiInfDer.x = bocaInfDer.x;
                roiInfDer.y = bocaInfIzq.y;
            }

            Rect roiBoca(roiSupIzq, roiInfDer);
            Mat boca(frame, roiBoca);

            if (ventanaValores.ui->cbRecortada->isChecked())
                this->frame = boca.clone();
        }




        if (ventanaValores.ui->cbActualizar->isChecked())  {
            umbralCannyMin = ventanaValores.ui->leUmbralHueMin->text().toFloat();
            umbralCannyMax = ventanaValores.ui->leUmbralHueMax->text().toFloat();
            tamanoApertura = ventanaValores.ui->leTamApertura->text().toFloat();
            L2Gradiente = ventanaValores.ui->cbL2Gradiente->isChecked();
        }



        if (ventanaValores.ui->cbLab->isChecked())  {

//            cvtColor( frame, frame, CV_BGR2HSV );
            cvtColor( frame, frame, CV_BGR2Lab );



//            int sumaComponente_a = 0;

//            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {
//                sumaComponente_a += (frame.data[i+COMP_Lab] - 128);  // Sumamos el componente 'a' de cada pixel
//            }

//            // Calculamos el promedio (segun paper que dice que es una forma de obtener el color de labio)
//            int mediaComponente_a = sumaComponente_a / (frame.cols*frame.rows);

//            // Un dato para el calculo de varianza
//            int sumatoriaDatoMenosPromedioAlCuadrado = 0;

//            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {
//                sumatoriaDatoMenosPromedioAlCuadrado += ( (frame.data[i+COMP_Lab] - 128) - mediaComponente_a )^2;
//            }

//            double varianza = sumatoriaDatoMenosPromedioAlCuadrado / (frame.cols*frame.rows);

////            qDebug() << "Promedio=" << mediaComponente_a << " Varianza=" << varianza;

//            // El paper 'a real-time lip localization...' dice que el umbral para detectar labios es con esta formula
//            float umbral = mediaComponente_a + varianza;

//            // Creamos una imagen gris
//            std::vector<uint8_t> labiosEnBinario;

//            // Aqui vamos completando cada pixel gris con valores negros y blancos
//            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {

//                // Restamos 128 porque el componente "a" tiene valores negativos y el frame va de 0 a 255 solamente
//                // Ver http://www.seas.upenn.edu/~bensapp/opencvdocs/ref/opencvref_cv.htm
//                if ( (frame.data[i+COMP_Lab] - 128) > umbral )  {
//                    labiosEnBinario.push_back(255);
//                }
//                else  {
//                    labiosEnBinario.push_back(0);
//                }
//            }




            unsigned int cantidadComponentes = frame.cols*frame.rows*3;

            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {
                if (frame.data[i + COMPONENTE_A] > ventanaValores.ui->leRojoDescartado->text().toInt())  {
                    frame.data[i + COMPONENTE] = 100;  // Sumamos el componente 'L' de cada pixel
                }
            }




            int componente_L = 0;




            int sumaComponente_x = 0;

            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {
                sumaComponente_x += ( frame.data[i + COMPONENTE] );  // Sumamos el componente 'L' de cada pixel
            }

            // Calculamos el promedio (segun paper que dice que es una forma de obtener el color de labio)
            int mediaComponente_x = sumaComponente_x / (frame.cols*frame.rows);

            // Un dato para el calculo de varianza
            int sumatoriaDatoMenosPromedioAlCuadrado = 0;

            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {
                sumatoriaDatoMenosPromedioAlCuadrado += ( (frame.data[i + COMPONENTE] ) - mediaComponente_x )^2;
            }

            double varianza = sumatoriaDatoMenosPromedioAlCuadrado / (frame.cols*frame.rows);

//            qDebug() << "Promedio=" << mediaComponente_a << " Varianza=" << varianza;

            // El paper 'a real-time lip localization...' dice que el umbral para detectar labios es con esta formula
            float umbral = mediaComponente_x + varianza;


            qDebug() << umbral;






            // El paper 'a real-time lip localization...' dice que el umbral para detectar labios es con esta formula
//            float umbral = ;

            // Creamos una imagen gris
            std::vector<uint8_t> labiosEnBinario;

            // Aqui vamos completando cada pixel gris con valores negros y blancos
            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {

                componente_L = (frame.data[i + COMPONENTE]);  // Obtenemos el componente 'hue' de cada pixel

                // Restamos 128 porque el componente "a" tiene valores negativos y el frame va de 0 a 255 solamente
                // Ver http://www.seas.upenn.edu/~bensapp/opencvdocs/ref/opencvref_cv.htm
//                if ( componente_L > ventanaValores.ui->leUmbralHueMin->text().toInt() &&
//                     componente_L < ventanaValores.ui->leUmbralHueMax->text().toInt() )  {
                if ( componente_L > umbral + ventanaValores.ui->leUmbralHueMin->text().toInt() )  {
                    labiosEnBinario.push_back(255);
                }
                else  {
                    labiosEnBinario.push_back(0);
                }
            }

            // Con ese vector pasamos a mat para luego llevar esa imagen en escala de grises a RGB
            Mat matLabiosEnBinario(Size(frame.cols, frame.rows), CV_8UC1, (void*)labiosEnBinario.data());
            cvtColor(matLabiosEnBinario, frame, CV_GRAY2BGR);
        }


        if (ventanaValores.ui->cbProcesar->isChecked())  {

            vector<vector<Point> > contours;
            vector<Vec4i> hierarchy;

            cvtColor( frame, frame, CV_BGR2GRAY );
            blur( frame, frame, Size(3,3) );

            Canny( frame, frame, umbralCannyMin, umbralCannyMax, tamanoApertura, L2Gradiente);
//            findContours( frame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
            findContours( frame, contours, hierarchy, CV_RETR_TREE, CV_LINK_RUNS, Point(0, 0) );

            int min = ventanaValores.ui->leContornoMin->text().toInt();
            vector<vector<Point> >::iterator iterContour;
            double length;
            iterContour = contours.begin();

            while(iterContour != contours.end())  {

                length = arcLength((*iterContour), true);
                if (length < min)  {
                    iterContour = contours.erase(iterContour);
                }
                else
                    iterContour++;
            }

            Mat maskContornos = Mat::zeros( frame.size(), CV_8UC1 );
            for (unsigned int i=0 ; i<contours.size() ; i++)  {
                drawContours(maskContornos, contours, i, Scalar(255, 0, 0), 1, 8, hierarchy, 0, Point());
            }

            cvtColor(maskContornos, frame, CV_GRAY2RGB);
//            cvtColor(frame, frame, CV_GRAY2RGB);
        }


        if (ventanaValores.ui->cbMedianFilter->isChecked())  {
            // Median smoothing
            medianBlur( frame, frame, ventanaValores.ui->leMedianFilter->text().toInt() );
        }



        if (ventanaValores.ui->cbHSI->isChecked())  {

            unsigned int cantidadComponentes = frame.cols*frame.rows*3;

            // Lo siguiente obtiene R G y B para luego obtener el componente I de HSI.

            unsigned char *input = (unsigned char*)(frame.data);

            unsigned int R=0, G=0, B=0, componente_I=0;

            std::vector<uint8_t> dientesEnBinario;

            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {
                B = input[i ];
                G = input[i + 1];
                R = input[i + 2];

                componente_I = (R+G+B)/3.0;

                if ( componente_I > (unsigned int)ventanaValores.ui->leUmbralHSI->text().toInt() )  {
                    dientesEnBinario.push_back(255);
                }
                else  {
                    dientesEnBinario.push_back(0);
                }
            }

            // Con ese vector pasamos a mat para luego llevar esa imagen en escala de grises a RGB
            Mat matDientesEnBinario(Size(frame.cols, frame.rows), CV_8UC1, (void*)dientesEnBinario.data());
            cvtColor(matDientesEnBinario, frame, CV_GRAY2BGR);
        }



        if (ventanaValores.ui->cbHistograma->isChecked())  {

            // Estas tres lineas ecualizan en gris
//            cvtColor( frame, frame, CV_BGR2GRAY );
//            equalizeHist( frame, frame );
//            cvtColor(frame, frame, CV_GRAY2BGR);

            // Lo siguiente ecualiza cada componente R, G y B pero no mantiene muy bien los colores originales
//            vector<Mat> channels;
//            split(frame,channels);
//            Mat B,G,R;

//            equalizeHist( channels[0], B );
//            equalizeHist( channels[1], G );
//            equalizeHist( channels[2], R );
//            vector<Mat> combined;
//            combined.push_back(B);
//            combined.push_back(G);
//            combined.push_back(R);

//            merge(combined, frame);


            // Esto segun> http://prateekvjoshi.com/2013/11/22/histogram-equalization-of-rgb-images/
//            Mat ycrcb;
//            cvtColor(frame,ycrcb,CV_BGR2YCrCb);

//            vector<Mat> channels;
//            split(ycrcb,channels);

//            equalizeHist(channels[0], channels[0]);

//            merge(channels,ycrcb);
//            cvtColor(ycrcb,frame,CV_YCrCb2BGR);






            // Segun http://www.dte.us.es/ing_inf/trat_voz/Practicas/Practica3.pdf vamos a hacer lo siguinete>
            // - Transformar la imagen RGB a HLS
            // - Aplicar el algoritmo de ecualización a la matriz L
            // - Invertir la imagen HLS a RGB

//            normalize(frame, frame, 0, 1, NORM_MINMAX);
            Mat hls;
            cvtColor(frame,hls,CV_BGR2HLS);

            vector<Mat> channels;
            split(hls,channels);

            equalizeHist(channels[1], channels[1]);

            merge(channels,hls);
            cvtColor(hls,frame,CV_HLS2BGR);



        }




        if (ventanaValores.ui->cbYUV->isChecked())  {

//            cvtColor(frame, frame, CV_BGR2YCrCb);
            cvtColor(frame, frame, CV_BGR2YUV);

//            QImage imageRGB(histImage.data, histImage.cols, histImage.rows, QImage::Format_RGB888);
//            visualizadorExtra.setHistograma(imageRGB);

//            if ( !visualizadorExtra.isVisible())  {
//                visualizadorExtra.show();
//                visualizadorExtra.move(1000, 0);
//            }


            unsigned int cantidadComponentes = frame.cols*frame.rows*3;

            unsigned char *input = (unsigned char*)(frame.data);

            unsigned int componente_Y=0;

            QVector<int> histogramaY(256, 0);
            QVector<uint8_t> compTodos;

            std::vector<uint8_t> dientesEnBinario;

            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {
                componente_Y = input[i];
                compTodos.push_back(componente_Y);

                histogramaY[componente_Y]++;
            }


            int sumaHistograma = 0;
            double mediaHistograma = 0;
            int sumatoriaDatoMenosPromedioAlCuadrado = 0;
            double varianza = 0, varianzaAnterior = 0;

            int posicionUmbral = 256;

            int inicioValores = histogramaY.size()-1;

            int ciclo = 1;

            while(ciclo<=254)  {

                for (int i=inicioValores ; i>=inicioValores-ciclo ; i--)  {
                    sumaHistograma += ( histogramaY.at(i) );
//                    qDebug() << "for sumaHistograma" << sumaHistograma << "valor" << histogramaY.at(i);
                }

                // Calculamos el promedio (segun paper que dice que es una forma de obtener el color de labio)
                mediaHistograma = sumaHistograma / (ciclo+1);

//                qDebug() << "promedio" << mediaHistograma;

                for (int i=inicioValores ; i>=inicioValores-ciclo ; i--)  {
                    sumatoriaDatoMenosPromedioAlCuadrado += qPow(histogramaY.at(i) - mediaHistograma, 2);
//                    qDebug() << "Segundo for - sumatoria" << sumatoriaDatoMenosPromedioAlCuadrado << "valor" << histogramaY.at(i);
                }

                varianza = sumatoriaDatoMenosPromedioAlCuadrado / (ciclo+1);

                if ( (varianza - varianzaAnterior) > ventanaValores.ui->leDiferenciaVarianzas->text().toInt())  {
                    posicionUmbral = inicioValores - ciclo;
//                    qDebug() << posicionUmbral;
                    break;
                }
                else  {
                    varianzaAnterior = sumatoriaDatoMenosPromedioAlCuadrado / (ciclo+1);
                }

//                qDebug() << "ciclo" << ciclo << "varianza" << varianza;

                ciclo++;
                sumaHistograma = 0;
                mediaHistograma = 0;
                sumatoriaDatoMenosPromedioAlCuadrado = 0;
                varianza = 0;

            }



            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {

                componente_Y = input[i];

                if ( (int)componente_Y > posicionUmbral - ventanaValores.ui->leUmbralMenosEsto->text().toInt())  {
                    dientesEnBinario.push_back(255);
                }
                else  {
                    dientesEnBinario.push_back(0);
                }
            }

//            qDebug() << "INICIO _________Umbral " << posicionUmbral;
//            qDebug() << "_________Histograma";
//            qDebug() << histogramaY;
//            qDebug() << "_________Valores intensidad";
//            qDebug() << compTodos;

            int anchoImagen = 256;
            int altoImagen = 500;

            Mat histogramY_Mat( 1, 256, CV_32S , histogramaY.data() );

//            float todoSumado = 0;
//            float elMayor = 0;

//            for( int i = 0; i < histogramaY.size(); i++ )  {
//                todoSumado += histogramaY.at(i);
//                if (elMayor < histogramaY.at(i))  {
//                    elMayor = histogramaY.at(i);
//                }
//            }

//            qDebug() << 1 - (elMayor/todoSumado);

            int min = 0;
            int max = 0;

            for( int i = 0; i < histogramaY.size(); i++ )  {
                if (histogramaY.at(i) != 0 && min==0)  {
                    min = i;
                }

                if (min!=0 && histogramaY.at(i) == 0 && max==0)  {
                    max = i;
                }
            }
            qDebug() << max-min;




            Mat histImage( altoImagen, anchoImagen, CV_8UC3, Scalar( 0,0,0) );

            int bin_w = cvRound( (double) anchoImagen/256 );

            for( int i = 1; i < 256; i++ )  {
                line( histImage, Point( bin_w*(i-1), altoImagen - cvRound(histogramY_Mat.at<int>(i-1)) ) ,
                             Point( bin_w*(i), altoImagen - cvRound(histogramY_Mat.at<int>(i)) ),
                             Scalar( 255, 0, 0), 2, 8, 0  );
            }


            QImage imageRGB(histImage.data, anchoImagen, altoImagen, QImage::Format_RGB888);

            visualizadorExtra.setHistograma(imageRGB, posicionUmbral);

            if ( !visualizadorExtra.isVisible())  {

//                visualizadorExtra.show();
//                visualizadorExtra.move(1000, 0);
            }



            // Con ese vector pasamos a mat para luego llevar esa imagen en escala de grises a RGB
            Mat matDientesEnBinario(Size(frame.cols, frame.rows), CV_8UC1, (void*)dientesEnBinario.data());
            cvtColor(matDientesEnBinario, frame, CV_GRAY2BGR);
        }



        if (ventanaValores.ui->cbHarris->isChecked())  {
            /// Detector parameters
            int blockSize = 2;
            int apertureSize = 3;
            double k = 0.04;
            cvtColor(frame, frame, CV_BGR2GRAY);
            cornerHarris( frame, frame, blockSize, apertureSize, k, BORDER_DEFAULT );
        }





        if (ventanaValores.ui->cbEliminarSkin->isChecked())  {
            cvtColor( frame, frame, CV_BGR2Lab );

            unsigned int cantidadComponentes = frame.cols*frame.rows*3;
            int sumaComponente_a = 0;

            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {
                sumaComponente_a += (frame.data[i+1] - 128);  // Sumamos el componente 'a' de cada pixel
            }

            // Calculamos el promedio (segun paper que dice que es una forma de obtener el color de labio)
            int mediaComponente_a = sumaComponente_a / (frame.cols*frame.rows);

            // Un dato para el calculo de varianza
            int sumatoriaDatoMenosPromedioAlCuadrado = 0;

            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {
                sumatoriaDatoMenosPromedioAlCuadrado += ( (frame.data[i+1] - 128) - mediaComponente_a )^2;
            }

            double varianza = sumatoriaDatoMenosPromedioAlCuadrado / (frame.cols*frame.rows);

            // El paper 'a real-time lip localization...' dice que el umbral para detectar labios es con esta formula
            float umbral = mediaComponente_a + varianza;

            // Aqui vamos completando cada pixel gris con valores negros y blancos
            for (unsigned int i=0 ; i<cantidadComponentes ; i+=3)  {

//                // Restamos 128 porque el componente "a" tiene valores negativos y el frame va de 0 a 255 solamente
//                // Ver http://www.seas.upenn.edu/~bensapp/opencvdocs/ref/opencvref_cv.htm
                if ( (frame.data[i+1] - 128) > umbral )  {
                    frame.data[i] = 0;
                    frame.data[i+1] = 0;
                    frame.data[i+2] = 0;
                }
            }

            cvtColor(frame, frame, CV_Lab2BGR);

        }









        ventana->escena->setImage(frame);
    }
}

float HiloCaptura::angulo(Point p1, Point p2)
{
    float adyacente = p2.x - p1.x;
    float opuesto;
    if (p2.y < p1.y) opuesto = p1.y - p2.y;
    else opuesto = p2.y - p1.y;
    float ang = atan(opuesto/adyacente);
//    qDebug() << "Angulo entre ojos en grados: " << ang*57.2957795 << "°";
    return ang;
}

Point HiloCaptura::inferiorDe(Point ojo, float ang, bool caso)
{
    int y = ojo.y + 400;
    int x;
    if(caso == 0) x = ojo.x + (tan(ang)*y);
    else x = ojo.x - (tan(ang)*y);
    return Point(x, y);
}

Point HiloCaptura::puntoNuevo(Point ojo, float ang, bool caso, int dist)
{
    int x;
    if (caso == 0) x =  ojo.x + sin(ang) * dist;
    else x =  ojo.x - sin(ang) * dist;
    int y = ojo.y + cos(ang) * dist;
    return Point(x, y);
}

void HiloCaptura::finalizarHilo()  {
    this->activo = false;
}

