/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_base.h"
#include "hdf_io_service.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"

int32_t HdfLoadDriverByServiceName(const char *serviceName)
{
    int32_t ret = HDF_FAILURE;
    struct HdfSBuf *data = NULL;
    if (serviceName == NULL || strcmp(serviceName, DEV_MGR_NODE) == 0) {
        return ret;
    }
    struct HdfIoService *ioService = HdfIoServiceBind(DEV_MGR_NODE);
    if (ioService == NULL) {
        HDF_LOGE("Fail to get %s service", DEV_MGR_NODE);
        return ret;
    }
    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        HDF_LOGE("fail to obtain sbuf data");
        ret = HDF_DEV_ERR_NO_MEMORY;
        goto out;
    }
    if (!HdfSbufWriteString(data, serviceName)) {
        HDF_LOGE("fail to write sbuf");
        ret = HDF_FAILURE;
        goto out;
    }
    ret = ioService->dispatcher->Dispatch(&ioService->object, DEVMGR_LOAD_SERVICE, data, NULL);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("fail to send service call");
    }
out:
    HdfIoServiceRecycle(ioService);
    HdfSBufRecycle(data);
    return ret;
}

int32_t HdfGetServiceNameByDeviceClass(DeviceClass deviceClass, struct HdfSBuf *reply)
{
    int32_t ret = HDF_FAILURE;
    struct HdfSBuf *data = NULL;
    if (reply == NULL) {
        HDF_LOGE("%s input reply is null", __func__);
        return ret;
    }
    struct HdfIoService *ioService = HdfIoServiceBind(DEV_MGR_NODE);
    if (ioService == NULL) {
        HDF_LOGE("Fail to get %s service", DEV_MGR_NODE);
        return ret;
    }
    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        HDF_LOGE("fail to obtain sbuf data");
        ret = HDF_DEV_ERR_NO_MEMORY;
        goto out;
    }
    if (!HdfSbufWriteInt32(data, deviceClass)) {
        HDF_LOGE("fail to write sbuf");
        ret = HDF_FAILURE;
        goto out;
    }
    ret = ioService->dispatcher->Dispatch(&ioService->object, DEVMGR_GET_SERVICE, data, reply);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("fail to send service call");
    }
out:
    HdfIoServiceRecycle(ioService);
    HdfSBufRecycle(data);
    return ret;
}

