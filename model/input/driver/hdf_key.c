/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_device_desc.h"
#include "osal_mem.h"
#include "gpio_if.h"
#include "hdf_log.h"
#include "hdf_input_device_manager.h"
#include "event_hub.h"
#include "hdf_key.h"

#define CHECK_PARSER_RET(ret,  str) do { \
    if (ret != HDF_SUCCESS) { \
        HDF_LOGE("%s: %s failed, ret = %d!", __func__, str, ret); \
        return HDF_FAILURE; \
    } \
} while (0)

static int32_t IoctlReadKeyEvent(KeyDriver *driver, unsigned long arg)
{
    KeyEventData *eventData = (KeyEventData *)(uintptr_t)arg;

    if (eventData == NULL) {
        HDF_LOGE("%s: param is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
#ifndef __KERNEL__
    if (LOS_ArchCopyToUser(eventData, &driver->eventData, sizeof(driver->eventData)) != 0) {
        HDF_LOGE("%s:copy chipInfo failed", __func__);
        return HDF_FAILURE;
    }
#endif
    return HDF_SUCCESS;
}

int32_t KeyIoctl(InputDevice *inputdev, int32_t cmd, unsigned long arg)
{
    int32_t ret;
    KeyDriver *driver = (KeyDriver *)inputdev->pvtData;
    switch (cmd) {
        case INPUT_IOCTL_GET_EVENT_DATA:
            ret = IoctlReadKeyEvent(driver, arg);
            break;
        default:
            ret = 0;
            HDF_LOGE("%s: cmd unknown, cmd = 0x%x", __func__, cmd);
            break;
    }
    return ret;
}

int32_t KeyIrqHandle(uint16_t intGpioNum, void *data)
{
    uint16_t gpioValue;
    KeyDriver *driver = (KeyDriver *)data;
    KeyEventData *event = &driver->eventData;
    int32_t ret = GpioDisableIrq(intGpioNum);

    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: disable irq failed, ret %d", __func__, ret);
    }

    ret = GpioRead(intGpioNum, &gpioValue);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gpio read failed, ret %d", __func__, ret);
        return HDF_FAILURE;
    }
    uint64_t curTime = OsalGetSysTimeMs();
    driver->preStatus = gpioValue;
    driver->timeStamp = curTime;

    if (gpioValue == GPIO_VAL_LOW) {
        event->definedEvent = INPUT_KEY_DOWN;
        input_report_key(driver->inputdev, KEY_POWER, 1);
    } else if (gpioValue == GPIO_VAL_HIGH) {
        event->definedEvent = INPUT_KEY_UP;
        input_report_key(driver->inputdev, KEY_POWER, 0);
    }
    input_sync(driver->inputdev);

    driver->dataHandledFlag = true;
    GpioEnableIrq(intGpioNum);
    return HDF_SUCCESS;
}

static int32_t SetupKeyIrq(KeyDriver *keyDrv)
{
    uint16_t intGpioNum = keyDrv->keyCfg->gpioNum;
    uint16_t irqFlag = keyDrv->keyCfg->irqFlag;
    int32_t ret = GpioSetDir(intGpioNum, GPIO_DIR_IN);

    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gpio set dir failed, ret %d", __func__, ret);
        return ret;
    }
    ret = GpioSetIrq(intGpioNum, irqFlag | GPIO_IRQ_USING_THREAD, KeyIrqHandle, keyDrv);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: register irq failed, ret %d", __func__, ret);
        return ret;
    }
    ret = GpioEnableIrq(intGpioNum);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: enable irq failed, ret %d", __func__, ret);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t KeyInit(KeyDriver *keyDrv)
{
    int32_t ret = SetupKeyIrq(keyDrv);
    CHECK_RETURN_VALUE(ret);
    return HDF_SUCCESS;
}

static KeyChipCfg *KeyConfigInstance(struct HdfDeviceObject *device)
{
    KeyChipCfg *keyCfg = (KeyChipCfg *)OsalMemAlloc(sizeof(KeyChipCfg));
    if (keyCfg == NULL) {
        HDF_LOGE("%s: malloc key config failed", __func__);
        return NULL;
    }
    (void)memset_s(keyCfg, sizeof(KeyChipCfg), 0, sizeof(KeyChipCfg));
    keyCfg->hdfKeyDev = device;

    if (ParseKeyConfig(device->property, keyCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: parse key config failed", __func__);
        OsalMemFree(keyCfg);
        keyCfg = NULL;
    }
    return keyCfg;
}

static KeyDriver *KeyDriverInstance(KeyChipCfg *keyCfg)
{
    KeyDriver *keyDrv = (KeyDriver *)OsalMemAlloc(sizeof(KeyDriver));
    if (keyDrv == NULL) {
        HDF_LOGE("%s: malloc key driver failed", __func__);
        return NULL;
    }
    (void)memset_s(keyDrv, sizeof(KeyDriver), 0, sizeof(KeyDriver));

    keyDrv->devType = keyCfg->devType;
    keyDrv->keyCfg = keyCfg;

    return keyDrv;
}

static InputDevice *InputDeviceInstance(KeyDriver *keyDrv)
{
    InputDevice *inputDev = (InputDevice *)OsalMemAlloc(sizeof(InputDevice));
    if (inputDev == NULL) {
        HDF_LOGE("%s: malloc input device failed", __func__);
        return NULL;
    }
    (void)memset_s(inputDev, sizeof(InputDevice), 0, sizeof(InputDevice));

    inputDev->pvtData = (void *)keyDrv;
    inputDev->devType = keyDrv->devType;
    inputDev->devName = keyDrv->keyCfg->keyName;
    inputDev->hdfDevObj = keyDrv->keyCfg->hdfKeyDev;
    keyDrv->inputdev = inputDev;

    return inputDev;
}

static int32_t RegisterKeyDevice(KeyChipCfg *keyCfg)
{
    KeyDriver *keyDrv = KeyDriverInstance(keyCfg);
    if (keyDrv == NULL) {
        HDF_LOGE("%s: instance key config failed", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

#ifdef __KERNEL__
    init_waitqueue_head(&keyDrv->pollWait);
#else
    __init_waitqueue_head(&keyDrv->pollWait);
#endif

    int32_t ret = KeyInit(keyDrv);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: key driver init failed, ret %d", __func__, ret);
        goto EXIT;
    }

    InputDevice *inputDev = InputDeviceInstance(keyDrv);
    if (inputDev == NULL) {
        HDF_LOGE("%s: instance input device failed", __func__);
        goto EXIT;
    }

    ret = RegisterInputDevice(inputDev);
    if (ret != HDF_SUCCESS) {
        goto EXIT1;
    }
    return HDF_SUCCESS;

EXIT1:
    OsalMemFree(inputDev->pkgBuf);
    OsalMemFree(inputDev);
EXIT:
    OsalMemFree(keyDrv);
    HDF_LOGE("%s: exit failed", __func__);
    return HDF_FAILURE;
}

static int32_t HdfKeyDriverInit(struct HdfDeviceObject *device)
{
    HDF_LOGI("%s: enter", __func__);
    if (device == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    KeyChipCfg *keyCfg = KeyConfigInstance(device);
    if (keyCfg == NULL) {
        HDF_LOGE("%s: instance key config failed", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    int32_t ret = RegisterKeyDevice(keyCfg);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }
    HDF_LOGI("%s: exit succ!", __func__);
    return HDF_SUCCESS;

EXIT:
    OsalMemFree(keyCfg);
    return HDF_FAILURE;
}

static int32_t HdfKeyDispatch(struct HdfDeviceIoClient *client, int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)cmd;
    if (client == NULL || data == NULL || reply == NULL) {
        HDF_LOGE("%s: param is null", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t HdfKeyDriverBind(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    static struct IDeviceIoService keyService = {
        .object.objectId = 1,
        .Dispatch = HdfKeyDispatch,
    };
    device->service = &keyService;
    return HDF_SUCCESS;
}

struct HdfDriverEntry g_hdfKeyEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_KEY",
    .Bind = HdfKeyDriverBind,
    .Init = HdfKeyDriverInit,
};

HDF_INIT(g_hdfKeyEntry);
