/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_device_info.h"
#include "hdf_log.h"
#include "osal_mem.h"

#define HDF_LOG_TAG device_info

void HdfDeviceInfoConstruct(struct HdfDeviceInfo *deviceInfo)
{
    if (deviceInfo == NULL) {
        return;
    }
    deviceInfo->isDynamic = false;
    deviceInfo->hostId = 0;
    deviceInfo->deviceId = 0;
    deviceInfo->policy = SERVICE_POLICY_INVALID;
    deviceInfo->priority = 0;
    deviceInfo->preload = DEVICE_PRELOAD_ENABLE;
    deviceInfo->permission = 0;
    deviceInfo->svcName = NULL;
    deviceInfo->moduleName = NULL;
    deviceInfo->deviceMatchAttr = NULL;
}

struct HdfDeviceInfo *HdfDeviceInfoNewInstance()
{
    struct HdfDeviceInfo *deviceInfo =
            (struct HdfDeviceInfo*)OsalMemCalloc(sizeof(struct HdfDeviceInfo));
    if (deviceInfo != NULL) {
        HdfDeviceInfoConstruct(deviceInfo);
        return deviceInfo;
    }
    HDF_LOGE("Create device deviceInfo failed");
    return NULL;
}

void HdfDeviceInfoFreeInstance(struct HdfDeviceInfo *deviceInfo)
{
    if (deviceInfo != NULL) {
        if (deviceInfo->isDynamic && deviceInfo->svcName != NULL) {
            OsalMemFree((void *)deviceInfo->svcName);
        }
        OsalMemFree(deviceInfo);
    }
}

void HdfDeviceInfoDelete(struct HdfSListNode *listEntry)
{
    struct HdfDeviceInfo *deviceInfo = (struct HdfDeviceInfo *)listEntry;
    HdfDeviceInfoFreeInstance(deviceInfo);
}

