#include "UpdateWorker.h"

#include <QUrl>
#include <QFile>
#include <QTimer>
#include <QEventLoop>
#include <QDataStream>
#include <QJsonObject>
#include <QApplication>
#include <QJsonDocument>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

UpdateWorker::UpdateWorker(QObject *parent) :
    QObject(parent), pending(false)
{
    connect(&t, &QTimer::timeout, [this]() {
        if (pending) {
            emit checkComplete("", tr("Time limit exceeded during version check. Please check your "
                                      "internet connection and try again."));
        }
    });
}

void UpdateWorker::checkCurrentVersion(time_t timeoutMs)
{
    QUrl url("https://api.github.com/repos/radareorg/cutter/releases/latest");
    QNetworkRequest request;
    request.setUrl(url);

    t.setInterval(timeoutMs);
    t.setSingleShot(true);
    t.start();

    connect(nm.get(request), &QNetworkReply::finished,
            this, &UpdateWorker::serveVersionCheckReply);
    pending = true;
}

void UpdateWorker::download(QDir downloadDir, QString version)
{

    QString downloadFileName;
#ifdef Q_OS_LINUX
    downloadFileName = "Cutter-v%1-x%2.Linux.AppImage";
#elif defined (Q_OS_WIN64) || defined (Q_OS_WIN32)
    downloadFileName = "Cutter-v%1-x%2.Windows.zip";
#elif defined (Q_OS_MACOS)
    downloadFileName = "Cutter-v%1-x%2.macOS.dmg";
#endif
    downloadFileName = downloadFileName
                       .arg(version)
                       .arg(QSysInfo::buildAbi().split('-').at(2).contains("64")
                            ? "64"
                            : "32");

    downloadFile.setFileName(downloadDir.filePath(downloadFileName));
    downloadFile.open(QIODevice::WriteOnly);

    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QUrl url(QString("https://github.com/radareorg/cutter/releases/"
                     "download/v%1/%2").arg(version).arg(downloadFileName));
    request.setUrl(url);

    QNetworkReply* reply = nm.get(request);
    downloadReply = reply;
    connect(downloadReply, &QNetworkReply::downloadProgress,
            this, &UpdateWorker::process);
    connect(downloadReply, &QNetworkReply::finished,
            this, &UpdateWorker::serveDownloadFinish);
}

void UpdateWorker::abortDownload()
{
    disconnect(downloadReply, &QNetworkReply::finished,
               this, &UpdateWorker::serveDownloadFinish);
    disconnect(downloadReply, &QNetworkReply::downloadProgress,
               this, &UpdateWorker::process);
    downloadReply->close();
    downloadReply->deleteLater();
    downloadFile.remove();
}

void UpdateWorker::serveVersionCheckReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    pending = false;
    QString currVersion = "";
    QString errStr = "";
    if (reply->error()) {
        errStr = reply->errorString();
    } else {
        currVersion = QJsonDocument::fromJson(reply->readAll()).object().value("tag_name").toString();
        currVersion.remove('v');
    }
    reply->close();
    reply->deleteLater();
    emit checkComplete(currVersion, errStr);
}

void UpdateWorker::serveDownloadFinish()
{
    downloadReply->close();
    downloadReply->deleteLater();
    emit downloadFinished(downloadFile.fileName());
}

void UpdateWorker::process(size_t bytesReceived, size_t bytesTotal)
{
    downloadFile.write(downloadReply->readAll());
    emit downloadProcess(bytesReceived, bytesTotal);
}
