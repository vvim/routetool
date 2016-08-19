#ifndef DISTANCE_MATRIX_H
#define DISTANCE_MATRIX_H

#include <QObject>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "globalfunctions.h"
#include "smarker.h"

#define DISTANCE_IN_METERS 1
#define DISTANCE_IN_SECONDS 2

#define DISTANCE_MATRIX_PART_AA 0
#define DISTANCE_MATRIX_PART_B 1
#define DISTANCE_MATRIX_PART_CC 2
#define DISTANCE_MATRIX_PART_D 3

#define DISTANCE_MATRIX_READY 4

extern QSettings settings;

class DistanceMatrix : public QObject
{
    Q_OBJECT
public:
    explicit DistanceMatrix(QObject *parent = 0);
    ~DistanceMatrix();

    void getDistances(QList <SMarker*> markers);
    void calculateOptimalRoute();

signals:
    void errorOccured(const QString&);
    void new_order_smarkers(QList<int> *);
    void new_distance_matrices(int**, int**);

private slots:
    void replyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_pNetworkAccessManager;
    QString buildQjsonUrl(QList <SMarker*> markers, int origins_start, int origins_end, int destinations_start, int destinations_end);
    QString buildQjsonUrl(QList <SMarker*> markers);

    // calculate distance in meters or in seconds?
    int distance_calc;

    QList <SMarker*> m_markers;

    // travelling salesman problem:
    int Vlaspit;
    //int size;
    int current_minimum_cost;
    int **distance_matrix_in_meters;
    int **distance_matrix_in_seconds;
    QList<int>* tsp_solution;
    QList<int>* all_cities_marked;
    QList<QString>* citynames;

    QString prepareForUrl(QString string);
    void deleteTheMatrices();

    // tsp help-functions
    int initialize_current_minimum_cost(int size);
    void fill_all_cities(int size);
    void reset_all_tsp(int nr_of_cities);
    void tsp(QList<int>* pad, int totale_kost, QList<int>* overschietende_steden);

    // A[10x10] == VulMatrixIn(QStringList_MAX_nr_of_cities)
    // B[ 5x10] == VulMatrixIn(QStringList_nr_of_cities-laatste5-arrive)
    // C[10x5 ] == VulMatrixIn(QStringList_nr_of_cities-laatste5-depart)
    // D[ 5x5 ] == VulMatrixIn(QStringList_nr_of_cities-laatste5-arrive-depart)
    //
    //                      AA   B
    //                      AA   B
    //  afstanden_matrix =     +
    //                      CC   D
    int distance_matrix_ready_to_process;
                              // if nr_of_cities < MAX_NR_OF_CITIES, this variable has no value
                              // else, this variable has to be DISTANCE_MATRIX_READY, before the distance_matrix is ready to be processed
                              // (to check if matrix AA B CC and D are combined before to continue)

    void logOutputCitynamesDistanceMatrices();
};

#endif // DISTANCE_MATRIX_H
