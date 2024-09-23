/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "pwm_core.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"

#define HDF_LOG_TAG pwm_core
#define PWM_NAME_LEN 32

static struct PwmDev *PwmGetDevByNum(uint32_t num)
{
    int ret;
    char *name = NULL;
    struct PwmDev *pwm = NULL;

    name = (char *)OsalMemCalloc(PWM_NAME_LEN + 1);
    if (name == NULL) {
        return NULL;
    }
    ret = snprintf_s(name, PWM_NAME_LEN + 1, PWM_NAME_LEN, "HDF_PLATFORM_PWM_%u", num);
    if (ret < 0) {
        HDF_LOGE("%s: snprintf_s failed", __func__);
        OsalMemFree(name);
        return NULL;
    }
    pwm = (struct PwmDev *)DevSvcManagerClntGetService(name);
    OsalMemFree(name);
    return pwm;
}

DevHandle PwmOpen(uint32_t num)
{
    int32_t ret;
    struct PwmDev *pwm = PwmGetDevByNum(num);

    if (pwm == NULL) {
        HDF_LOGE("%s: dev is null", __func__);
        return NULL;
    }
    (void)OsalSpinLock(&(pwm->lock));
    if (pwm->busy) {
        (void)OsalSpinUnlock(&(pwm->lock));
        HDF_LOGE("%s: pwm%u is busy", __func__, num);
        return NULL;
    }
    if (pwm->method != NULL && pwm->method->open != NULL) {
        ret = pwm->method->open(pwm);
        if (ret != HDF_SUCCESS) {
            (void)OsalSpinUnlock(&(pwm->lock));
            HDF_LOGE("%s: open failed, ret %d", __func__, ret);
            return NULL;
        }
    }
    pwm->busy = true;
    (void)OsalSpinUnlock(&(pwm->lock));
    return (DevHandle)pwm;
}

void PwmClose(DevHandle handle)
{
    int32_t ret;
    struct PwmDev *pwm = (struct PwmDev *)handle;

    if (pwm == NULL) {
        HDF_LOGE("%s: dev is null", __func__);
        return;
    }

    if (pwm->method != NULL && pwm->method->close != NULL) {
        ret = pwm->method->close(pwm);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: close failed, ret %d", __func__, ret);
            return;
        }
    }
    (void)OsalSpinLock(&(pwm->lock));
    pwm->busy = false;
    (void)OsalSpinUnlock(&(pwm->lock));
}

int32_t PwmSetConfig(DevHandle handle, struct PwmConfig *config)
{
    int32_t ret;
    struct PwmDev *pwm = NULL;

    HDF_LOGI("%s: enter.", __func__);
    if (handle == NULL) {
        HDF_LOGE("%s: handle is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (config == NULL) {
        HDF_LOGE("%s: config is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    pwm = (struct PwmDev *)handle;
    if (memcmp(config, &(pwm->cfg), sizeof(*config)) == 0) {
        HDF_LOGE("%s: do not need to set config", __func__);
        return HDF_SUCCESS;
    }
    if (pwm->method == NULL || pwm->method->setConfig == NULL) {
        HDF_LOGE("%s: setConfig is not support", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }
    HDF_LOGI("%s: set PwmConfig: number %u, period %u, duty %u, polarity %u, enable %u.", __func__,
        config->number, config->period, config->duty, config->polarity, config->status);
    ret = pwm->method->setConfig(pwm, config);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: failed, ret %d", __func__, ret);
        return ret;
    }
    pwm->cfg = *config;
    HDF_LOGI("%s: success.", __func__);

    return HDF_SUCCESS;
}

int32_t PwmGetConfig(DevHandle handle, struct PwmConfig *config)
{
    struct PwmDev *pwm = NULL;

    HDF_LOGI("%s: enter.", __func__);
    if (handle == NULL) {
        HDF_LOGE("%s: handle is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (config == NULL) {
        HDF_LOGE("%s: config is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    pwm = (struct PwmDev *)handle;
    *config = pwm->cfg;
    HDF_LOGI("%s: success.", __func__);

    return HDF_SUCCESS;
}

int32_t PwmEnable(DevHandle handle)
{
    struct PwmConfig config;
    uint32_t curValue;
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (PwmGetConfig(handle, &config) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    curValue = config.status;
    config.status = PWM_ENABLE_STATUS;
    ret = PwmSetConfig(handle, &config);
    if (ret == HDF_SUCCESS) {
        HDF_LOGI("%s: success. enable: %d -> %d.", __func__, curValue, config.status);
    }
    return ret;
}

int32_t PwmDisable(DevHandle handle)
{
    struct PwmConfig config;
    uint32_t curValue;
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (PwmGetConfig(handle, &config) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    curValue = config.status;
    config.status = PWM_DISABLE_STATUS;
    ret = PwmSetConfig(handle, &config);
    if (ret == HDF_SUCCESS) {
        HDF_LOGI("%s: success. enable: %d -> %d.", __func__, curValue, config.status);
    }
    return ret;
}

int32_t PwmSetPeriod(DevHandle handle, uint32_t period)
{
    struct PwmConfig config;
    uint32_t curValue;
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (PwmGetConfig(handle, &config) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    curValue = config.period;
    config.period = period;
    ret = PwmSetConfig(handle, &config);
    if (ret == HDF_SUCCESS) {
        HDF_LOGI("%s: success. period: %d -> %d.", __func__, curValue, config.period);
    }
    return ret;
}

int32_t PwmSetDuty(DevHandle handle, uint32_t duty)
{
    struct PwmConfig config;
    uint32_t curValue;
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (PwmGetConfig(handle, &config) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    curValue = config.duty;
    config.duty = duty;
    ret = PwmSetConfig(handle, &config);
    if (ret == HDF_SUCCESS) {
        HDF_LOGI("%s: success. duty: %d -> %d.", __func__, curValue, config.duty);
    }
    return ret;
}

int32_t PwmSetPolarity(DevHandle handle, uint8_t polarity)
{
    struct PwmConfig config;
    uint32_t curValue;
    int32_t ret;

    HDF_LOGI("%s: enter.", __func__);
    if (PwmGetConfig(handle, &config) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    curValue = config.polarity;
    config.polarity = polarity;
    ret = PwmSetConfig(handle, &config);
    if (ret == HDF_SUCCESS) {
        HDF_LOGI("%s: success. polarity: %d -> %d.", __func__, curValue, config.polarity);
    }
    return ret;
}

int32_t PwmSetPriv(struct PwmDev *pwm, void *priv)
{
    if (pwm == NULL) {
        HDF_LOGE("%s: pwm is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    pwm->priv = priv;
    return HDF_SUCCESS;
}

void *PwmGetPriv(struct PwmDev *pwm)
{
    if (pwm == NULL) {
        HDF_LOGE("%s: pwm is null", __func__);
        return NULL;
    }
    return pwm->priv;
}

int32_t PwmDeviceAdd(struct HdfDeviceObject *obj, struct PwmDev *pwm)
{
    if (obj == NULL || pwm == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (pwm->method == NULL || pwm->method->setConfig == NULL) {
        HDF_LOGE("%s: setConfig is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (OsalSpinInit(&(pwm->lock)) != HDF_SUCCESS) {
        HDF_LOGE("%s: init spinlock fail", __func__);
        return HDF_FAILURE;
    }
    pwm->device = obj;
    obj->service = &(pwm->service);
    return HDF_SUCCESS;
}

void PwmDeviceRemove(struct HdfDeviceObject *obj, struct PwmDev *pwm)
{
    if (obj == NULL || pwm == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return;
    }
    (void)OsalSpinDestroy(&(pwm->lock));
    pwm->device = NULL;
    obj->service = NULL;
}
