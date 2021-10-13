#ifndef DECOMPILER_H
#define DECOMPILER_H

#include "CutterCommon.h"
#include "RizinTask.h"
#include <rz_util/rz_annotated_code.h>

#include <QString>
#include <QObject>

/**
 * Implements a decompiler that can be registered using CutterCore::registerDecompiler()
 */
class CUTTER_EXPORT Decompiler : public QObject
{
    Q_OBJECT

private:
    const QString id;
    const QString name;

public:
    Decompiler(const QString &id, const QString &name, QObject *parent = nullptr);
    ~Decompiler() override;

    static RzAnnotatedCode *makeWarning(QString warningMessage);

    QString getId() const { return id; }
    QString getName() const { return name; }
    virtual bool isRunning() { return false; }
    virtual bool isCancelable() { return false; }

    virtual void decompileAt(RVA addr) = 0;
    virtual void cancel() {}

signals:
    void finished(RzAnnotatedCode *codeDecompiled);
};

class JSDecDecompiler : public Decompiler
{
    Q_OBJECT

private:
    RizinCmdTask *task;

public:
    explicit JSDecDecompiler(QObject *parent = nullptr);
    ~JSDecDecompiler() override;

    void decompileAt(RVA addr) override;

    bool isRunning() override { return task != nullptr; }

    static bool isAvailable();
};

#endif // DECOMPILER_H
