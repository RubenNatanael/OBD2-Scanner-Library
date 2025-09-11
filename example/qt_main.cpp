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

    OBD2Scanner obd("vcan0");

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
            auto res = obd.getPid(0x00);
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
            auto res = obd.getFreezFrame(03);
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
