#include "core/OperationResult.hpp"

OperationResult OperationResult::ok(const QString& message)
{
    OperationResult result;
    result.success = true;
    result.message = message;
    return result;
}

OperationResult OperationResult::fail(const QString& message, int errorCode)
{
    OperationResult result;
    result.success = false;
    result.message = message;
    result.errorCode = errorCode;
    return result;
}
