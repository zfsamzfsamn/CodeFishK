/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_log.h"
#include "osal_case_cmd_test.h"

int OsalGetTestResult(uint32_t cmd)
{
    HDF_LOGD("[OSAL_UT_TEST]%s %u start", __func__, cmd);
    return OSAL_TEST_CASE_CHECK(cmd);
}

