#include "AboutWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QTextEdit>
#include <QScrollArea>
#include <QFrame>
#include <QPixmap>
#include <QFile>

AboutWindow::AboutWindow(const QString &version, QWidget *parent): QDialog(parent) {
    setWindowTitle("About Sensory Guard");
    setWindowModality(Qt::NonModal);
    setFixedSize(390, 350);
    setupUi(version);
}

void AboutWindow::setupUi(const QString &version) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Header = logo + name + version
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    QLabel *logoLabel = new QLabel();
    logoLabel->setPixmap(QPixmap(":/icons/logo.png").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setFixedSize(48, 48);
    logoLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(logoLabel);

    QVBoxLayout *nameLayout = new QVBoxLayout();
    QLabel *nameLabel = new QLabel("Sensory Guard");
    nameLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    nameLayout->addWidget(nameLabel);
    
    QLabel *versionLabel = new QLabel("Version v" + version);
    versionLabel->setStyleSheet("color: gray; font-size: 11px;");
    nameLayout->addWidget(versionLabel);
    nameLayout->addStretch();
    
    headerLayout->addLayout(nameLayout);
    headerLayout->addStretch();
    
    QWidget *headerWidget = new QWidget();
    headerWidget->setLayout(headerLayout);
    headerWidget->setFixedHeight(80);
    mainLayout->addWidget(headerWidget);

    QTabWidget *tabWidget = new QTabWidget(this);

    // About tab
    QWidget *aboutWidget = new QWidget();
    QVBoxLayout *aboutLayout = new QVBoxLayout(aboutWidget);

    QLabel *descLabel = new QLabel();
    descLabel->setText("Sensory Guard is an offline, real-time NSFW screen censor application that uses an on-device machine learning model to detect and censor sensitive content live on the screen.");
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 12px;");
    aboutLayout->addWidget(descLabel);

    QLabel *websiteLabel = new QLabel("Website: <a href='https://github.com/khde/sensory-guard' style='color: #0066cc;'>https://github.com/khde/sensory-guard</a>");
    websiteLabel->setOpenExternalLinks(true);
    websiteLabel->setStyleSheet("font-size: 11px; margin-top: 12px;");
    websiteLabel->setWordWrap(true);
    aboutLayout->addWidget(websiteLabel);

    aboutLayout->addStretch();
    tabWidget->addTab(aboutWidget, "About");

    // License tab
    QWidget *licenseWidget = new QWidget();
    QVBoxLayout *licenseLayout = new QVBoxLayout(licenseWidget);

    QTextEdit *licenseText = new QTextEdit();
    licenseText->setReadOnly(true);
    QFile licenseFile(":/LICENSE");
    if (licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        licenseText->setPlainText(QString::fromUtf8(licenseFile.readAll()));
    }
    licenseLayout->addWidget(licenseText);
    tabWidget->addTab(licenseWidget, "License");

    // Components tab
    QWidget *componentsWidget = new QWidget();
    QVBoxLayout *componentsLayout = new QVBoxLayout(componentsWidget);

    const struct { const char* name; const char* desc; } components[] = {
        {"Qt", "Cross-platform GUI and application framework"},
        {"ONNX Runtime", "Cross-platform machine learning inference engine"},
        {"OpenCV", "Open-source computer vision software library"},
        {"Lucide", "Open-source icon library"}
    };

    for (size_t i = 0; i < sizeof(components) / sizeof(components[0]); ++i) {
        QLabel *compLabel = new QLabel(components[i].name);
        compLabel->setStyleSheet("font-weight: bold;");
        componentsLayout->addWidget(compLabel);

        QLabel *compDescLabel = new QLabel(components[i].desc);
        compDescLabel->setStyleSheet("color: gray; font-size: 10px;");
        componentsLayout->addWidget(compDescLabel);

        if (i < sizeof(components) / sizeof(components[0]) - 1) {
            QFrame *separator = new QFrame();
            separator->setFrameShape(QFrame::HLine);
            separator->setFrameShadow(QFrame::Sunken);
            separator->setStyleSheet("margin: 8px 0;");
            componentsLayout->addWidget(separator);
        }
    }

    componentsLayout->addStretch();
    
    QScrollArea *componentsScrollArea = new QScrollArea();
    componentsScrollArea->setWidget(componentsWidget);
    componentsScrollArea->setWidgetResizable(true);
    tabWidget->addTab(componentsScrollArea, "Components");

    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);
}
