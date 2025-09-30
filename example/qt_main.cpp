#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QSplitter>
#include <QMessageBox>
#include <QLineEdit>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include "../include/OBD2Scanner.h"

// ---------------- Helpers ----------------
static QString vectorToQString(const std::vector<DecodedItem>& vec) {
    QString result;
    for (const auto &s : vec)
        result += QString::fromStdString(s.label) + ": " + QString::fromStdString(s.value) + "\n";
    return result.trimmed();
}

// Retry wrapper
void requestWithRetry(std::function<void(std::function<void(const std::vector<DecodedItem>&)>)> request,
                      int retries,
                      std::function<void(const std::vector<DecodedItem>&)> onResult)
{
    request([=](const std::vector<DecodedItem>& res) mutable {
        bool needRetry = false;
        if (res.empty()) needRetry = true;
        if (res.size() == 1 && (res[0].value == "-1" || res[0].value == "No response")) needRetry = true;
        if (needRetry && retries > 0) {
            requestWithRetry(request, retries - 1, onResult);
        } else {
            if (onResult) onResult(res);
        }
    });
}

// ---------------- Main ----------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("OBD2 Qt Scanner");

    QVBoxLayout *mainLayout = new QVBoxLayout;

    // ----- Status / VIN / Protocol -----
    QPushButton *buttonConnection = new QPushButton("Connect to Device");
    QLabel *statusLabel = new QLabel("Starting...");
    QLabel *vinLabel = new QLabel("VIN: N/A");
    QLabel *protoLabel = new QLabel("Protocol: N/A");

    QWidget *topWidget = new QWidget;
    QVBoxLayout *topLayout = new QVBoxLayout(topWidget);
    topLayout->addWidget(buttonConnection);
    topLayout->addWidget(statusLabel);
    topLayout->addWidget(vinLabel);
    topLayout->addWidget(protoLabel);
    topLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(topWidget);

    // ----- Splitter Left/Right -----
    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    // Left panel: PID info
    QWidget *leftWidget = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);

    QScrollArea *leftScroll = new QScrollArea;
    leftScroll->setWidgetResizable(true);
    leftScroll->setWidget(leftWidget);
    leftScroll->setMinimumHeight(1);

    splitter->addWidget(leftScroll);
    splitter->setStretchFactor(0, 3);  // Left panel

    // Right panel: DTCs
    QWidget *rightWidget = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);

    QScrollArea *rightScroll = new QScrollArea;
    rightScroll->setWidgetResizable(true);
    rightScroll->setWidget(rightWidget);

    splitter->addWidget(rightScroll);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter);
    mainLayout->setStretchFactor(topWidget, 0);
    mainLayout->setStretchFactor(splitter, 1);

    // ----- Transport / Scanner -----
    std::string interfaceName = "vcan0"; // or "/dev/rfcomm0"
    ICANInterface* transport = new SocketCAN();
    //ICANInterface* transport = new ELM327Transport(B38400, '6');
    transport->setTimeout(500);

    OBD2Scanner obd(transport);

    // ----- PID Storage -----
    std::vector<uint8_t> allPIDs; // store all PIDs discovered
    QComboBox *pidCombo = new QComboBox;


    // ----- Async Init -----
    QtConcurrent::run([&]() {
        QMetaObject::invokeMethod(statusLabel, [=]() { statusLabel->setText("Connecting..."); });
        bool ok = transport->init(interfaceName);
        if (!ok) {
            QMetaObject::invokeMethod(statusLabel, [=]() { statusLabel->setText("Failed to connect"); });
            return;
        }

        // Protocol
        std::string proto = obd.getProtocol();
        QMetaObject::invokeMethod(protoLabel, [=]() { protoLabel->setText("Protocol: " + QString::fromStdString(proto)); });

        // VIN
        QMetaObject::invokeMethod(statusLabel, [=]() { statusLabel->setText("Retrieving VIN..."); });
        requestWithRetry([&](auto cb){ obd.getVehicleInfo(0x02, cb); }, 3,
                         [&](const std::vector<DecodedItem>& vin){
            QMetaObject::invokeMethod(vinLabel, [=]() { vinLabel->setText("VIN:\n" + vectorToQString(vin)); });
        });

        // Discover PIDs: blocks 0x00, 0x20, 0x40, 0x60
        QMetaObject::invokeMethod(statusLabel, [=]() { statusLabel->setText("Retrieving supported PIDs..."); });
        std::vector<uint8_t> pidBlocks = {0x00, 0x20, 0x40, 0x60};
        for (uint8_t block : pidBlocks) {
            requestWithRetry([&](auto cb){ obd.getPid(block, cb); }, 3,
                             [&](const std::vector<DecodedItem>& blockRes){
                bool flag = true;
                for (const auto &item : blockRes) {
                    uint8_t pid = static_cast<uint8_t>(std::stoi(item.value));
                    for (const auto &pidBlock: pidBlocks) {
                        if (pid == pidBlock)
                            flag = false;
                    }
                    if (flag) {
                        allPIDs.push_back(pid);
                        pidCombo->addItem("PID " + QString::number(pid,16), QString::number(pid));
                    }
                }
            });
        }

        QMetaObject::invokeMethod(statusLabel, [=]() { statusLabel->setText("Ready"); });
    });

    // ----- Top Panel UI ------
    QObject::connect(buttonConnection, &QPushButton::clicked, [statusLabel, &obd, &transport, interfaceName]() {
        statusLabel->setText("Connecting...");
        QtConcurrent::run([statusLabel, &transport, interfaceName]() {
            bool ok = transport->init(interfaceName); 
            // Update GUI in main thread
            QMetaObject::invokeMethod(statusLabel, [statusLabel, ok]() {
                 statusLabel->setText(ok ? "Connected" : "Disconnected"); 
            }); 
        }); 
    });

    // ----- Left Panel UI -----
    // Request all PIDs button
    QPushButton *allPidBtn = new QPushButton("Request All PID Data");
    leftLayout->addWidget(allPidBtn);
    // Scroll area for PID responses
    QWidget *scrollContent = new QWidget;
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setAlignment(Qt::AlignTop);

    QScrollArea *pidScroll = new QScrollArea;
    pidScroll->setWidgetResizable(true);
    pidScroll->setWidget(scrollContent);

    leftLayout->addWidget(allPidBtn);
    leftLayout->addWidget(pidScroll, 7);

    // ComboBox + single PID button
    QPushButton *pidBtn = new QPushButton("Get PID Data");
    QLabel *pidResLabel = new QLabel("No data yet");

    QHBoxLayout *pidRow = new QHBoxLayout;
    pidRow->addWidget(pidCombo);
    pidRow->addWidget(pidBtn);
    leftLayout->addLayout(pidRow, 3);
    leftLayout->addWidget(pidResLabel);

    // Update the "Request All PID Data" logic to add labels to scrollLayout
    QObject::connect(allPidBtn, &QPushButton::clicked, [=, &obd, &allPIDs, scrollLayout]() {
        // Clear previous PID labels
        QLayoutItem *child;
        while ((child = scrollLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }

        for (uint8_t pid : allPIDs) {
            requestWithRetry([&](auto cb){ obd.getPid(pid, cb); }, 3,
                            [=, &scrollLayout](const std::vector<DecodedItem>& res){
                QMetaObject::invokeMethod(scrollLayout->parentWidget(), [=, &scrollLayout]() {
                    QLabel *pidLabel = new QLabel("PID " + QString::number(pid,16) + ":\n" + vectorToQString(res));
                    scrollLayout->addWidget(pidLabel);
                });
            });
        }
    });

    // Button click: request single PID
    QObject::connect(pidBtn, &QPushButton::clicked, [=, &obd, &pidCombo, &pidResLabel, &allPIDs]() {
        int idx = pidCombo->currentIndex();
        if (idx < 0) return;

        // Use selected PID from comboBox (must be from allPIDs vector)
        uint8_t pid = static_cast<uint8_t>(std::stoi(pidCombo->currentData().toString().toStdString()));

        requestWithRetry([&](auto cb){ obd.getPid(pid, cb); }, 3,
                        [=](const std::vector<DecodedItem>& res){
            pidResLabel->setText(vectorToQString(res));
        });
    });

    // ----- Right Panel UI -----
    // Right panel scroll area for responses (50% height)

    rightScroll->setWidgetResizable(true);
    rightScroll->setWidget(rightWidget);
    splitter->addWidget(rightScroll);
    splitter->setStretchFactor(1, 2); // 50% width

    // --- DTC Buttons ---
    struct DTCButtonInfo {
        QString title;
        std::function<void(std::function<void(const std::vector<DecodedItem>&)>)> requestFn;
    };

    std::vector<DTCButtonInfo> dtcButtons = {
        {"Get DTCs", [&](auto cb){ obd.getDTCs(cb); }},
        {"Get Pending DTCs", [&](auto cb){ obd.getPendingDTCs(cb); }},
        {"Get Permanent DTCs", [&](auto cb){ obd.getPermanentDTCs(cb); }}
    };

    for (auto &btnInfo : dtcButtons) {
        QHBoxLayout *row = new QHBoxLayout;
        QPushButton *btn = new QPushButton(btnInfo.title);
        QLabel *label = new QLabel("No data yet");
        row->addWidget(btn);
        row->addWidget(label);
        rightLayout->addLayout(row);

        QObject::connect(btn, &QPushButton::clicked, [=, &obd]() {
            requestWithRetry(btnInfo.requestFn, 3, [=](const std::vector<DecodedItem>& res){
                QMetaObject::invokeMethod(label, [label, res](){
                    label->setText(vectorToQString(res));
                });
            });
        });
    }

    // --- Freeze Frame: request all PIDs from allPIDs vector ---
    QPushButton *freezeBtn = new QPushButton("Get Freeze Frame");
    rightLayout->addWidget(freezeBtn);
    QObject::connect(freezeBtn, &QPushButton::clicked, [=, &obd, &allPIDs, rightLayout]() {
        // Clear previous PID labels in scroll area except top buttons
        QLayoutItem *child;
        while ((child = rightLayout->takeAt(3)) != nullptr) { // keep top 3 rows
            if (child->widget()) delete child->widget();
            delete child;
        }

        for (uint8_t pid : allPIDs) {
            requestWithRetry([&](auto cb){ obd.getFreezFrame(pid, cb); }, 3,
                            [=](const std::vector<DecodedItem>& res){
                QMetaObject::invokeMethod(rightLayout->parentWidget(), [rightLayout, pid, res]() {
                    QLabel *pidLabel = new QLabel("PID " + QString::number(pid,16) + ":\n" + vectorToQString(res));
                    rightLayout->addWidget(pidLabel);
                });
            });
        }
    });

    // Clear DTC button
    QPushButton *clearBtn = new QPushButton("Clear DTCs (Danger!)");
    clearBtn->setStyleSheet("background-color:red;color:white;");
    rightLayout->addWidget(clearBtn);
    QObject::connect(clearBtn, &QPushButton::clicked, [=, &obd, &rightLayout]() {
        if (QMessageBox::warning(nullptr, "Confirm",
                                 "Are you sure? This will erase all DTCs and Freeze Frames!",
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            auto res = obd.ClearDTCs();
            QLabel *doneLabel = new QLabel("DTCs cleared:\n" + vectorToQString(res));
            rightLayout->addWidget(doneLabel);
        }
    });

    // ----- Raw command (bottom of window) -----
    QHBoxLayout *rawRow = new QHBoxLayout;
    QLineEdit *rawEdit = new QLineEdit;
    QPushButton *rawBtn = new QPushButton("Send Raw Command");
    rawRow->addWidget(rawEdit);
    rawRow->addWidget(rawBtn);
    mainLayout->addLayout(rawRow);

    window.setLayout(mainLayout);
    window.show();

    return app.exec();
}
