#include <stdio.h>
#include "OpenCLUtil.h"

int main()
{
    OpenClAdapter _adapter;
    _adapter.Init();

    // Step 5: Create program object
    string sourceStr;
    std::string file = "";
    BYTEVIEW_COMMON::PathUtil::GetAbsDirPath("SimpleEncryptKernel.cl", file);
    file = "/Users/zengjj/Product/pc-sdk-fixbug/byteview-crypto/src/SimpleEncryptKernel.cl";
    cl_int status = _adapter.convertToString(file.c_str(), sourceStr);

    _adapter.StartTask("ElemOp", sourceStr.c_str());
    _adapter.StopTask();
    _adapter.Uninit();
    return 0;
}