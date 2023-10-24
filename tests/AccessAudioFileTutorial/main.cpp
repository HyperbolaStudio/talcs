#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>

#include "device/AudioDevice.h"
#include "device/AudioDriver.h"
#include "device/AudioDriverManager.h"
#include "device/AudioSourcePlayback.h"
#include "format/AudioFormatIO.h"
#include "format/AudioFormatInputSource.h"
#include "format/AudioSourceWriter.h"
#include "source/TransportAudioSource.h"

using namespace talcs;

QMainWindow *win;
QLabel *filenameLabel;
QPushButton *startPlayButton;
QPushButton *stopPlayButton;
QPushButton *exportAudioButton;

QFile *inputFile;
AudioFormatIO *inputFormatIO;
AudioFormatInputSource *inputSource;
TransportAudioSource *tpSrc;
AudioSourcePlayback *srcPlayback;

AudioDevice *dev;

void startPlay() {
    if (!dev->start(srcPlayback)) {
        QMessageBox::critical(win, "Error", "Cannot start playing!");
        return;
    }
    tpSrc->play();
    startPlayButton->setDisabled(true);
    stopPlayButton->setDisabled(false);
}

void stopPlay() {
    dev->stop();
    startPlayButton->setDisabled(false);
    stopPlayButton->setDisabled(true);
    exportAudioButton->setDisabled(false);
}

void selectFile() {
    auto filename = QFileDialog::getOpenFileName(win, {}, {}, "*.flac");
    if (filename.isEmpty())
        return;

    stopPlay();
    filenameLabel->setText(filename);
    inputFile->setFileName(filename);
    tpSrc->setPosition(0);
}

void exportAudio() {
    auto exportFileName = QFileDialog::getSaveFileName(win, {}, {}, "*.wav");
    if (exportFileName.isEmpty())
        return;
    stopPlay();

    inputSource->setResamplerMode(talcs::AudioFormatInputSource::SincBestQuality);

    QFile exportFile(exportFileName);
    AudioFormatIO exportIO(&exportFile);
    exportIO.open(QFile::WriteOnly, AudioFormatIO::WAV | AudioFormatIO::PCM_24, 2, 44100);
    tpSrc->open(4096, 44100);
    tpSrc->setPosition(0);
    tpSrc->play();

    QDialog dlg;
    dlg.setWindowTitle("Exporting...");
    auto dlgLayout = new QVBoxLayout;
    auto exportProgressBar = new QProgressBar;
    dlgLayout->addWidget(exportProgressBar);
    dlg.setLayout(dlgLayout);

    QThread thread;
    AudioSourceWriter writer(tpSrc, &exportIO, inputSource->length());
    writer.moveToThread(&thread);
    QObject::connect(&thread, &QThread::started, &writer, &AudioSourceWriter::start);
    QObject::connect(&writer, &AudioSourceWriter::blockProcessed, exportProgressBar, [=](qint64 sampleCountProcessed) {
        exportProgressBar->setValue(sampleCountProcessed * 100 / inputSource->length());
    });
    QObject::connect(&dlg, &QDialog::rejected, &thread, [&]() { writer.interrupt(); });
    QObject::connect(&writer, &AudioSourceWriter::finished, &dlg,[&] {
        if(writer.status() == talcs::AudioSourceProcessorBase::Completed)
            dlg.accept();
        else
            QMessageBox::warning(win, "Export", "Exporting is interrupted.");
    });

    thread.start();
    dlg.exec();
    tpSrc->close();
    inputSource->setResamplerMode(talcs::AudioFormatInputSource::SincFastest);

    thread.quit();
    thread.wait();
}

bool initializeAudioEngine() {
    inputFile = new QFile;
    inputFormatIO = new AudioFormatIO(inputFile);
    inputSource = new AudioFormatInputSource(inputFormatIO);
    tpSrc = new TransportAudioSource(inputSource);
    srcPlayback = new AudioSourcePlayback(tpSrc);

    inputSource->setResamplerMode(talcs::AudioFormatInputSource::SincFastest);

    auto drvMgr = AudioDriverManager::createBuiltInDriverManager();
    auto drv = drvMgr->driver(drvMgr->drivers()[0]);
    if (!drv->initialize()) {
        qCritical() << "Cannot initialize audio driver!";
        return false;
    }
    dev = drv->createDevice(drv->defaultDevice());
    if (!dev || !dev->isInitialized()) {
        qCritical() << "Cannot initialize audio device!";
        return false;
    }
    if (!dev->open(dev->preferredBufferSize(), dev->preferredSampleRate())) {
        qCritical() << "Cannot open audio device!";
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    QApplication a(argc, argv);

    filenameLabel = new QLabel;
    auto selectFileButton = new QPushButton("Select File");
    startPlayButton = new QPushButton("Start Playing");
    stopPlayButton = new QPushButton("Stop Playing");
    exportAudioButton = new QPushButton("Export Audio");

    startPlayButton->setDisabled(true);
    stopPlayButton->setDisabled(true);
    exportAudioButton->setDisabled(true);

    auto layout = new QVBoxLayout;
    layout->addWidget(filenameLabel);
    layout->addWidget(selectFileButton);
    layout->addWidget(startPlayButton);
    layout->addWidget(stopPlayButton);
    layout->addWidget(exportAudioButton);

    auto centralWidget = new QWidget;
    centralWidget->setLayout(layout);
    win = new QMainWindow;
    win->setCentralWidget(centralWidget);

    QObject::connect(selectFileButton, &QPushButton::clicked, &selectFile);
    QObject::connect(startPlayButton, &QPushButton::clicked, &startPlay);
    QObject::connect(stopPlayButton, &QPushButton::clicked, &stopPlay);
    QObject::connect(exportAudioButton, &QPushButton::clicked, &exportAudio);

    if (!initializeAudioEngine())
        return 1;

    win->show();

    return a.exec();
}