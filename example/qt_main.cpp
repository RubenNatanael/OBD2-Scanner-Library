#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QtConcurrent/QtConcurrent>
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

    std::string interfaceName = "vcan0"; // /dev/rfcomm0

    // Using socket CAN
    ICANInterface* transport = new SocketCAN();
    // Using ELM327 protocol
    //ICANInterface* transport = new ELM327Transport(B38400, '0');
    
    std::string connection = transport->init(interfaceName)? "Connected": "Disconnected";
    OBD2Scanner obd(transport);

    // --- Button 0: Init ---
    {
        QHBoxLayout *row = new QHBoxLayout;
        QPushButton *button = new QPushButton("Connect to Device");
        QLabel *label = new QLabel(QString::fromStdString(connection));
        row->addWidget(button);
        row->addWidget(label);

        QObject::connect(button, &QPushButton::clicked, [label, &obd, &connection, &transport, interfaceName]() {
            label->setText("Connecting...");

            QtConcurrent::run([label, &connection, &transport, interfaceName]() {
                bool ok = transport->init(interfaceName);

                // Update GUI in main thread
                QMetaObject::invokeMethod(label, [label, ok]() {
                    label->setText(ok ? "Connected" : "Disconnected");
                });
            });
        });


        mainLayout->addLayout(row);
    }

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

    // Shared combo box
    QComboBox *comboBox = new QComboBox();

    // --- Button 2: Get supported PIDs ---
    {
        QHBoxLayout *row = new QHBoxLayout;
        QPushButton *button = new QPushButton("Get supported PIDs");
        QLabel *label = new QLabel("No data yet");

        row->addWidget(button);
        row->addWidget(label);

        QObject::connect(button, &QPushButton::clicked, [label, comboBox, &obd]() {
            QtConcurrent::run([label, comboBox, &obd]() {
                obd.getPid(OBD2::Pid::Supported0, [label, comboBox](const std::vector<DecodedItem>& res) {
                    QMetaObject::invokeMethod(label, [label, comboBox, res]() {
                        label->setText(vectorToQString(res));

                        comboBox->clear();
                        for (const auto &item : res) {
                            comboBox->addItem(
                                QString("PID %1").arg(item.value.c_str()),
                                QVariant::fromValue(item.value) // store real enum
                            );
                        }
                    });
                });
            });
        });

        mainLayout->addLayout(row);
    }

    // --- Button 2.5: Get Info ---
    {
        QHBoxLayout *row = new QHBoxLayout;
        QPushButton *button = new QPushButton("Get Info Car");
        QLabel *label = new QLabel("No data yet");

        row->addWidget(button);
        row->addWidget(comboBox); // reuse the same one
        row->addWidget(label);

        QObject::connect(button, &QPushButton::clicked, [label, comboBox, &obd]() {
            int index = comboBox->currentIndex();
            if (index < 0) return;

            // Retrieve real PID enum directly
            uint8_t pid = static_cast<uint8_t>(std::stoi(comboBox->currentData().value<std::string>()));
            LOG_INFO(std::to_string(pid));

            QtConcurrent::run([label, pid, &obd]() {
                obd.getPid(pid, [label](const std::vector<DecodedItem>& res) {
                    QMetaObject::invokeMethod(label, [label, res]() {
                        label->setText(vectorToQString(res));
                    });
                });
            });
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
            auto res = obd.getFreezFrame(OBD2::Pid::CoolantTemp);
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
