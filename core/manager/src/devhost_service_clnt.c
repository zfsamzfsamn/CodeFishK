/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "device_token_clnt.h"
#include "devhost_service_clnt.h"
#include "devmgr_service_start.h"
#include "hdf_base.h"
#include "hdf_driver_installer.h"
#include "hdf_log.h"
#include "osal_mem.h"

#define HDF_LOG_TAG devhost_service_clnt

int DevHostServiceClntInstallDriver(struct DevHostServiceClnt *hostClnt)
{
    int ret;
    struct HdfSListIterator it;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct IDevHostService *devHostSvcIf = NULL;
    if (hostClnt == NULL) {
        HDF_LOGE("Install driver failed, hostClnt is null");
        return HDF_FAILURE;
    }

    devHostSvcIf = (struct IDevHostService *)hostClnt->hostService;
    if (devHostSvcIf == NULL || devHostSvcIf->AddDevice == NULL) {
        HDF_LOGE("devHostSvcIf or devHostSvcIf->AddDevice is null");
        return HDF_FAILURE;
    }
    HdfSListIteratorInit(&it, hostClnt->deviceInfos);
    while (HdfSListIteratorHasNext(&it)) {
        deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&it);
        if ((deviceInfo == NULL) || (deviceInfo->preload == DEVICE_PRELOAD_DISABLE)) {
            continue;
        }
        if ((DeviceManagerIsQuickLoad() == DEV_MGR_QUICK_LOAD) &&
            (deviceInfo->preload == DEVICE_PRELOAD_ENABLE_STEP2)) {
            continue;
        }
        ret = devHostSvcIf->AddDevice(devHostSvcIf, deviceInfo);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("Install %s driver failed, ret = %d", deviceInfo->svcName, ret);
        }
    }
    return HDF_SUCCESS;
}

static void DevHostServiceClntConstruct(struct DevHostServiceClnt *hostClnt)
{
    HdfSListInit(&hostClnt->devices);
}

struct DevHostServiceClnt *DevHostServiceClntNewInstance(uint16_t hostId, const char *hostName)
{
    struct DevHostServiceClnt *hostClnt =
        (struct DevHostServiceClnt *)OsalMemCalloc(sizeof(struct DevHostServiceClnt));
    if (hostClnt != NULL) {
        hostClnt->hostId = hostId;
        hostClnt->hostName = hostName;
        hostClnt->devCount = 0;
        DevHostServiceClntConstruct(hostClnt);
    }
    return hostClnt;
}

void DevHostServiceClntFreeInstance(struct DevHostServiceClnt *hostClnt)
{
    if (hostClnt != NULL) {
        HdfSListFlush(&hostClnt->devices, DeviceTokenClntDelete);
        HdfSListFlush(hostClnt->deviceInfos, HdfDeviceInfoDelete);
        OsalMemFree(hostClnt);
    }
}

void DevHostServiceClntDelete(struct HdfSListNode *listEntry)
{
    struct DevHostServiceClnt *hostClnt = (struct DevHostServiceClnt *)listEntry;
    if (hostClnt != NULL) {
        DevHostServiceClntFreeInstance(hostClnt);
    }
}

