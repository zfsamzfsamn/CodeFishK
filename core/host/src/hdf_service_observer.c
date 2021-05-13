/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_service_observer.h"
#include "hdf_base.h"
#include "hdf_cstring.h"
#include "hdf_log.h"
#include "hdf_observer_record.h"

#define HDF_LOG_TAG service_observer

bool HdfServiceObserverConstruct(struct HdfServiceObserver *observer)
{
    if (observer == NULL) {
        HDF_LOGE("observer is null");
        return false;
    }
    if (OsalMutexInit(&observer->observerMutex) != HDF_SUCCESS) {
        return false;
    }
    HdfSListInit(&observer->services);
    return true;
}

void HdfServiceObserverDestruct(struct HdfServiceObserver *observer)
{
    if (observer != NULL) {
        HdfSListFlush(&observer->services, HdfServiceObserverRecordDelete);
        OsalMutexDestroy(&observer->observerMutex);
        observer->observerMutex.realMutex = NULL;
    }
}

int HdfServiceObserverSubscribeService(struct HdfServiceObserver *observer,
    const char *svcName, uint32_t matchId, struct SubscriberCallback callback)
{
    struct HdfServiceObserverRecord *serviceRecord = NULL;
    struct HdfServiceSubscriber *subscriber = NULL;
    uint32_t serviceKey = HdfStringMakeHashKey(svcName, 0);
    if ((observer == NULL) || (svcName == NULL)) {
        HDF_LOGE("observer or svcName or callback.OnServiceConnected is null");
        return HDF_FAILURE;
    }
    serviceRecord = (struct HdfServiceObserverRecord *)HdfSListSearch(
        &observer->services, serviceKey, HdfServiceObserverRecordCompare);
    if (serviceRecord == NULL) {
        serviceRecord = HdfServiceObserverRecordObtain(serviceKey);
        if (serviceRecord == NULL) {
            HDF_LOGE("SubscribeService failed, serviceRecord is null");
            return HDF_FAILURE;
        }
        subscriber = HdfServiceSubscriberObtain(callback, matchId);
        if (subscriber == NULL) {
            HDF_LOGE("SubscribeService failed, subscriber is null");
            HdfServiceObserverRecordRecycle(serviceRecord);
            return HDF_FAILURE;
        }
        HdfSListAdd(&observer->services, &serviceRecord->entry);
    } else {
        subscriber = HdfServiceSubscriberObtain(callback, matchId);
        if (subscriber == NULL) {
            HDF_LOGE("SubscribeService failed, hdf search list is null, subscriber is null");
            return HDF_FAILURE;
        }
    }
    if ((serviceRecord->publisher != NULL) &&
        (subscriber->callback.OnServiceConnected != NULL) &&
        ((serviceRecord->policy != SERVICE_POLICY_PRIVATE) ||
        (serviceRecord->matchId == matchId))) {
        subscriber->state = HDF_SUBSCRIBER_STATE_READY;
        subscriber->callback.OnServiceConnected(subscriber->callback.deviceObject, serviceRecord->publisher);
    }
    OsalMutexLock(&serviceRecord->obsRecMutex);
    HdfSListAdd(&serviceRecord->subscribers, &subscriber->entry);
    OsalMutexUnlock(&serviceRecord->obsRecMutex);
    return HDF_SUCCESS;
}

int HdfServiceObserverPublishService(struct HdfServiceObserver *observer,
    const char *svcName, uint32_t matchId, uint16_t policy, struct HdfObject *service)
{
    struct HdfServiceObserverRecord *serviceRecord = NULL;
    uint32_t serviceKey = HdfStringMakeHashKey(svcName, 0);
    if ((observer == NULL) || (svcName == NULL)) {
        HDF_LOGE("observer or svcName is null");
        return HDF_FAILURE;
    }
    serviceRecord = (struct HdfServiceObserverRecord *)HdfSListSearch(
        &observer->services, serviceKey, HdfServiceObserverRecordCompare);
    if (serviceRecord == NULL) {
        serviceRecord = HdfServiceObserverRecordObtain(serviceKey);
        if (serviceRecord == NULL) {
            HDF_LOGE("PublishService failed, serviceRecord is null");
            return HDF_FAILURE;
        }
        serviceRecord->publisher = service;
        serviceRecord->matchId = matchId;
        serviceRecord->policy = policy;
        HdfSListAdd(&observer->services, &serviceRecord->entry);
    } else {
        serviceRecord->publisher = service;
        HdfServiceObserverRecordNotifySubscribers(serviceRecord, matchId, policy);
    }
    return HDF_SUCCESS;
}

void HdfServiceObserverRemoveRecord(struct HdfServiceObserver *observer, const char *svcName)
{
    struct HdfServiceObserverRecord *serviceRecord = NULL;
    uint32_t serviceKey = HdfStringMakeHashKey(svcName, 0);
    if ((observer == NULL) || (svcName == NULL)) {
        HDF_LOGW("observer or svcName is null");
        return;
    }
    serviceRecord = (struct HdfServiceObserverRecord *)HdfSListSearch(
        &observer->services, serviceKey, HdfServiceObserverRecordCompare);
    if (serviceRecord != NULL) {
        OsalMutexLock(&observer->observerMutex);
        HdfSListRemove(&observer->services, &serviceRecord->entry);
        OsalMutexUnlock(&observer->observerMutex);
        HdfServiceObserverRecordRecycle(serviceRecord);
    }
}

