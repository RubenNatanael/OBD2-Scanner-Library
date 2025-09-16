#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include "../include/OBD2Scanner.h"

static QString vectorToQString(const std::vector<DecodedItem>& vec) {
    QString result;
    for (const auto &s : vec) {
        result += QString::fromStdString(s.label) + " " + QString::fromStdString(s.value) + "\n";
    }
    return result.trimmed();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("OBD2 Qt Example");

    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Using socket CAN
    ICANInterface* transport = new SocketCAN();
    transport->init("vcan0");

    // Using ELM327 protocol
    /* ICANInterface* transport = new ELM327Transport();
    if (!transport->init("/dev/pts/3")) {
        exit(1);
    } */
    OBD2Scanner obd(transport);


    // --- Button 1: Get DTCs ---
    {
        QHBoxLayout *row = new QHBoxLayout;
        QPushButton *button = new QPushButton("Get DTCs");
        QLabel *label = new QLabel("No data yet");
        row->addWidget(button);
        row->addWidget(label);

        QObject::connect(button, &QPushButton::clicked, [label, &obd]() {
            auto res = obd.getDTCs();
            label->setText(vectorToQString(res));
        });

        mainLayout->addLayout(row);
    }

    // --- Button 2: Get supported PIDs ---
    {
        QHBoxLayout *row = new QHBoxLayout;
        QPushButton *button = new QPushButton("Get supported PIDs");
        QLabel *label = new QLabel("No data yet");
        row->addWidget(button);
        row->addWidget(label);

        QObject::connect(button, &QPushButton::clicked, [label, &obd]() {
            auto res = obd.getPid(OBD2::Pid::Supported0);
            label->setText(vectorToQString(res));
        });

        mainLayout->addLayout(row);
    }

    // --- Button 3: Freeze Frame ---
    {
        QHBoxLayout *row = new QHBoxLayout;
        QPushButton *button = new QPushButton("Get Freeze Frame");
        QLabel *label = new QLabel("No data yet");
        row->addWidget(button);
        row->addWidget(label);

        QObject::connect(button, &QPushButton::clicked, [label, &obd]() {
            auto res = obd.getFreezFrame(OBD2::Pid::FuelSystemStatus);
            label->setText(vectorToQString(res));
        });

        mainLayout->addLayout(row);
    }

    // --- Button 4: Clear DTC ---
    {
        QHBoxLayout *row = new QHBoxLayout;
        QPushButton *button = new QPushButton("Clear DTC");
        QLabel *label = new QLabel("No data yet");
        row->addWidget(button);
        row->addWidget(label);

        QObject::connect(button, &QPushButton::clicked, [label, &obd]() {
            auto res = obd.ClearDTCs();
            label->setText(vectorToQString(res));
        });

        mainLayout->addLayout(row);
    }

    window.setLayout(mainLayout);
    window.show();

    return app.exec();
}
