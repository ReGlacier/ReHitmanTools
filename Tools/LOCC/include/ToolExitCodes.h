#pragma once

namespace LOCC
{
    enum ToolExitCodes : int
    {
        Success = 0,

        BadSourceFile = -10,
        BadSourceFormat = -11,
        FailedToSerialize = -12,
        FailedToSaveSerializedResult = -13,
        FailedToLoadJson = -14,
        CompileError = -15,
        UnknownCompileError = -16,
        FailedToSaveCompiledResult = -17
    };
}