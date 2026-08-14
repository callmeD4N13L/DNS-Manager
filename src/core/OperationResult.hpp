#pragma once

#include <QMetaType>
#include <QString>

// Structured result of a backend operation. Every privileged or system-level
// call returns one of these so callers can report failures meaningfully.
struct OperationResult
{
    bool success = false;
    QString message;
    int errorCode = 0; // Windows GetLastError() / platform error code

    static OperationResult ok(const QString& message = {});
    static OperationResult fail(const QString& message, int errorCode = 0);
};

Q_DECLARE_METATYPE(OperationResult)