#pragma once

#include <QString>

// Application logging. installMessageHandler() replaces the Qt message
// handler with one that appends timestamped lines to a rotating file under
// <AppData>/logs/dnsmanager.log (max ~512 KB, previous file kept as .old).
namespace Logging {

QString defaultLogDir();
QString defaultLogFilePath();
void installMessageHandler();

} // namespace Logging