/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_pwm_entry_test.h"
#include "hdf_log.h"
#include "pwm_test.h"

#define HDF_LOG_TAG hdf_pwm_entry_test

int32_t HdfPwmUnitTestEntry(HdfTestMsg *msg)
{
    struct PwmTest *test = NULL;

    if (msg == NULL) {
        HDF_LOGE("%s: msg is NULL!", __func__);
        return HDF_FAILURE;
    }
    test = GetPwmTest();
    if (test == NULL || test->TestEntry == NULL) {
        HDF_LOGE("%s: test or TestEntry is NULL!", __func__);
        msg->result = HDF_FAILURE;
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: call [TestEntry]", __func__);
    msg->result = test->TestEntry(test, msg->subCmd);
    return msg->result;
}
