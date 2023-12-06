/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <iostream>
#include <thread>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"
#include "c_test/struct_test/v1_0/client/istruct_test.h"

using namespace OHOS;
using namespace testing::ext;

static struct IStructTest* g_testClient = nullptr;

class CStructTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase(){}
    void SetUp(){}
    void TearDown(){}
};

void CStructTest::SetUpTestCase()
{
    g_testClient = HdiStructTestGet();
    if (g_testClient == nullptr) {
        printf("CStructTest: get g_testClient failed.\n");
    }
}

void TearDownTestCase()
{
    if (g_testClient != nullptr) {
        HdiStructTestRelease(g_testClient);
        g_testClient = nullptr;
    }
}

HWTEST_F(CStructTest, CStructTest_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, g_testClient);
}

HWTEST_F(CStructTest, CStructTest_002, TestSize.Level1)
{
    struct SSample* srcObj = (struct SSample*)OsalMemCalloc(sizeof(struct SSample));
    ASSERT_NE(srcObj, nullptr);

    srcObj->m1 = true;
    srcObj->m2 = 1;
    srcObj->m3 = 10.125;
    srcObj->m4 = strdup("hello world");

    struct SSample* destObj = nullptr;
    int32_t ec = g_testClient->SSampleTest(g_testClient, srcObj, &destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);

    EXPECT_EQ((srcObj->m1 ? 1 : 0), (destObj->m1 ? 1 : 0));
    EXPECT_EQ(srcObj->m2, destObj->m2);
    EXPECT_DOUBLE_EQ(srcObj->m3, destObj->m3);
    EXPECT_STREQ(srcObj->m4, destObj->m4);

    SSampleFree(srcObj, true);
    SSampleFree(destObj, true);
}

HWTEST_F(CStructTest, CStructTest_003, TestSize.Level1)
{
    struct SSample2* srcObj = (struct SSample2*)OsalMemCalloc(sizeof(struct SSample2));
    ASSERT_NE(srcObj, nullptr);

    srcObj->m1 = true;
    srcObj->m2 = 65;
    srcObj->m3 = 10;
    srcObj->m4 = 20;
    srcObj->m5 = 30;
    srcObj->m6 = 97;
    srcObj->m7 = 100;
    srcObj->m8 = 200;
    srcObj->m9 = 300;
    srcObj->m10 = 10.5;
    srcObj->m11 = 20.125;

    struct SSample2* destObj = nullptr;
    int32_t ec = g_testClient->SSample2Test(g_testClient, srcObj, &destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);

    EXPECT_EQ((srcObj->m1 ? 1 : 0), (destObj->m1 ? 1 : 0));
    EXPECT_EQ(srcObj->m2, destObj->m2);
    EXPECT_EQ(srcObj->m3, destObj->m3);
    EXPECT_EQ(srcObj->m4, destObj->m4);
    EXPECT_EQ(srcObj->m5, destObj->m5);
    EXPECT_EQ(srcObj->m6, destObj->m6);
    EXPECT_EQ(srcObj->m7, destObj->m7);
    EXPECT_EQ(srcObj->m8, destObj->m8);
    EXPECT_EQ(srcObj->m9, destObj->m9);
    EXPECT_FLOAT_EQ(srcObj->m10, destObj->m10);
    EXPECT_DOUBLE_EQ(srcObj->m11, destObj->m11);

    SSample2Free(srcObj, true);
    SSample2Free(destObj, true);
}

HWTEST_F(CStructTest, CStructTest_004, TestSize.Level1)
{
    struct SSample3* srcObj = (struct SSample3*)OsalMemCalloc(sizeof(struct SSample3));
    ASSERT_NE(srcObj, nullptr);

    srcObj->m1 = strdup("hello world");
    srcObj->m2 = MEM_THREE;

    srcObj->m3.m1 = true;
    srcObj->m3.m2 = 65;
    srcObj->m3.m3 = 10;
    srcObj->m3.m4 = 20;
    srcObj->m3.m5 = 30;
    srcObj->m3.m6 = 97;
    srcObj->m3.m7 = 100;
    srcObj->m3.m8 = 200;
    srcObj->m3.m9 = 300;
    srcObj->m3.m10 = 10.5;
    srcObj->m3.m11 = 20.125;

    srcObj->m4 = open("/fdtest1.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);

    struct SSample3* destObj = nullptr;
    int32_t ec = g_testClient->SSample3Test(g_testClient, srcObj, &destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);

    EXPECT_STREQ(srcObj->m1, destObj->m1);
    EXPECT_EQ(srcObj->m2, destObj->m2);

    EXPECT_EQ((srcObj->m3.m1 ? 1 : 0), (destObj->m3.m1 ? 1 : 0));
    EXPECT_EQ(srcObj->m3.m2, destObj->m3.m2);
    EXPECT_EQ(srcObj->m3.m3, destObj->m3.m3);
    EXPECT_EQ(srcObj->m3.m4, destObj->m3.m4);
    EXPECT_EQ(srcObj->m3.m5, destObj->m3.m5);
    EXPECT_EQ(srcObj->m3.m6, destObj->m3.m6);
    EXPECT_EQ(srcObj->m3.m7, destObj->m3.m7);
    EXPECT_EQ(srcObj->m3.m8, destObj->m3.m8);
    EXPECT_EQ(srcObj->m3.m9, destObj->m3.m9);
    EXPECT_FLOAT_EQ(srcObj->m3.m10, destObj->m3.m10);
    EXPECT_DOUBLE_EQ(srcObj->m3.m11, destObj->m3.m11);

    SSample3Free(srcObj, true);
    SSample3Free(destObj, true);
}

HWTEST_F(CStructTest, CStructTest_005, TestSize.Level1)
{
    struct SSample4* srcObj = (struct SSample4*)OsalMemCalloc(sizeof(struct SSample4));
    ASSERT_NE(srcObj, nullptr);

    srcObj->m1Len = 2;
    srcObj->m1 = (bool*)OsalMemCalloc(sizeof(bool) * srcObj->m1Len);
    for (uint32_t i = 0; i < srcObj->m1Len; i++) {
        srcObj->m1[i] = true;
    }

    srcObj->m2Len = 2;
    srcObj->m2 = (int8_t*)OsalMemCalloc(sizeof(int8_t) * srcObj->m2Len);
    for (uint32_t i = 0; i < srcObj->m2Len; i++) {
        srcObj->m2[i] = 65;
    }

    srcObj->m3Len = 2;
    srcObj->m3 = (int16_t*)OsalMemCalloc(sizeof(int16_t) * srcObj->m3Len);
    for (uint32_t i = 0; i < srcObj->m3Len; i++) {
        srcObj->m3[i] = 3;
    }

    srcObj->m4Len = 2;
    srcObj->m4 = (int32_t*)OsalMemCalloc(sizeof(int32_t) * srcObj->m4Len);
    for (uint32_t i = 0; i < srcObj->m4Len; i++) {
        srcObj->m4[i] = 4;
    }

    srcObj->m5Len = 2;
    srcObj->m5 = (int64_t*)OsalMemCalloc(sizeof(int64_t) * srcObj->m5Len);
    for (uint32_t i = 0; i < srcObj->m5Len; i++) {
        srcObj->m5[i] = 5;
    }

    srcObj->m6Len = 2;
    srcObj->m6 = (uint8_t*)OsalMemCalloc(sizeof(uint8_t) * srcObj->m6Len);
    for (uint32_t i = 0; i < srcObj->m6Len; i++) {
        srcObj->m6[i] = 97;
    }

    srcObj->m7Len = 2;
    srcObj->m7 = (uint16_t*)OsalMemCalloc(sizeof(uint16_t) * srcObj->m7Len);
    for (uint32_t i = 0; i < srcObj->m7Len; i++) {
        srcObj->m7[i] = 7;
    }

    srcObj->m8Len = 2;
    srcObj->m8 = (uint32_t*)OsalMemCalloc(sizeof(uint32_t) * srcObj->m8Len);
    for (uint32_t i = 0; i < srcObj->m8Len; i++) {
        srcObj->m8[i] = 8;
    }

    srcObj->m9Len = 2;
    srcObj->m9 = (uint64_t*)OsalMemCalloc(sizeof(uint64_t) * srcObj->m9Len);
    for (uint32_t i = 0; i < srcObj->m9Len; i++) {
        srcObj->m9[i] = 9;
    }

    srcObj->m10Len = 2;
    srcObj->m10 = (float*)OsalMemCalloc(sizeof(float) * srcObj->m10Len);
    for (uint32_t i = 0; i < srcObj->m10Len; i++) {
        srcObj->m10[i] = 10.5;
    }

    srcObj->m11Len = 2;
    srcObj->m11 = (double*)OsalMemCalloc(sizeof(double) * srcObj->m11Len);
    for (uint32_t i = 0; i < srcObj->m11Len; i++) {
        srcObj->m11[i] = 11.55;
    }

    srcObj->m12Len = 2;
    srcObj->m12 = (char**)OsalMemCalloc(sizeof(char*) * srcObj->m12Len);
    for (uint32_t i = 0; i < srcObj->m12Len; i++) {
        const char* str = "hello world";
        srcObj->m12[i] = strdup(str);
    }

    srcObj->m13Len = 2;
    srcObj->m13 = (enum ESample*)OsalMemCalloc(sizeof(enum ESample) * srcObj->m13Len);
    for (uint32_t i = 0; i < srcObj->m13Len; i++) {
        srcObj->m13[i] = MEM_ONE;
    }

    srcObj->m14Len = 2;
    srcObj->m14 = (struct SSample*)OsalMemCalloc(sizeof(struct SSample) * srcObj->m14Len);
    for (uint32_t i = 0; i < srcObj->m14Len; i++) {
        srcObj->m14[i].m1 = true;
        srcObj->m14[i].m2 = 2;
        srcObj->m14[i].m3 = 3.55;
        srcObj->m14[i].m4 = strdup("hello");
    }

    struct SSample4* destObj = nullptr;
    int32_t ec = g_testClient->SSample4Test(g_testClient, srcObj, &destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);


    for (uint32_t i = 0; i < srcObj->m1Len; i++) {
        EXPECT_EQ((srcObj->m1[i] ? 1 : 0), (destObj->m1[i] ? 1 : 0));
    }

    for (uint32_t i = 0; i < srcObj->m2Len; i++) {
        EXPECT_EQ(srcObj->m2[i], destObj->m2[i]);
    }

    for (uint32_t i = 0; i < srcObj->m3Len; i++) {
        EXPECT_EQ(srcObj->m3[i], destObj->m3[i]);
    }

    for (uint32_t i = 0; i < srcObj->m4Len; i++) {
        EXPECT_EQ(srcObj->m4[i], destObj->m4[i]);
    }

    for (uint32_t i = 0; i < srcObj->m5Len; i++) {
        EXPECT_EQ(srcObj->m5[i], destObj->m5[i]);
    }

    for (uint32_t i = 0; i < srcObj->m6Len; i++) {
        EXPECT_EQ(srcObj->m6[i], destObj->m6[i]);
    }

    for (uint32_t i = 0; i < srcObj->m7Len; i++) {
        EXPECT_EQ(srcObj->m7[i], destObj->m7[i]);
    }

    for (uint32_t i = 0; i < srcObj->m8Len; i++) {
        EXPECT_EQ(srcObj->m8[i], destObj->m8[i]);
    }

    for (uint32_t i = 0; i < srcObj->m9Len; i++) {
        EXPECT_EQ(srcObj->m9[i], destObj->m9[i]);
    }

    for (uint32_t i = 0; i < srcObj->m10Len; i++) {
        EXPECT_FLOAT_EQ(srcObj->m10[i], destObj->m10[i]);
    }

    for (uint32_t i = 0; i < srcObj->m11Len; i++) {
        EXPECT_DOUBLE_EQ(srcObj->m11[i], destObj->m11[i]);
    }

    for (uint32_t i = 0; i < srcObj->m12Len; i++) {
        EXPECT_STREQ(srcObj->m12[i], destObj->m12[i]);
    }

    for (uint32_t i = 0; i < srcObj->m13Len; i++) {
        EXPECT_EQ(srcObj->m13[i], destObj->m13[i]);
    }

    for (uint32_t i = 0; i < srcObj->m14Len; i++) {
        EXPECT_EQ(((srcObj->m14[i]).m1 ? 1 : 0), ((destObj->m14[i]).m1 ? 1 : 0));
        EXPECT_EQ((srcObj->m14[i]).m2, (destObj->m14[i]).m2);
        EXPECT_DOUBLE_EQ((srcObj->m14[i]).m3, (destObj->m14[i]).m3);
        EXPECT_STREQ((srcObj->m14[i]).m4, (destObj->m14[i]).m4);
    }

    SSample4Free(srcObj, true);
    SSample4Free(destObj, true);
}

HWTEST_F(CStructTest, CStructTest_006, TestSize.Level1)
{
    struct SSample5* srcObj = (struct SSample5*)OsalMemCalloc(sizeof(struct SSample5));
    ASSERT_NE(srcObj, nullptr);

    srcObj->m1Len = 2;
    srcObj->m1 = (bool*)OsalMemCalloc(sizeof(bool) * srcObj->m1Len);
    for (uint32_t i = 0; i < srcObj->m1Len; i++) {
        srcObj->m1[i] = true;
    }

    srcObj->m2Len = 2;
    srcObj->m2 = (int8_t*)OsalMemCalloc(sizeof(int8_t) * srcObj->m2Len);
    for (uint32_t i = 0; i < srcObj->m2Len; i++) {
        srcObj->m2[i] = 65;
    }

    srcObj->m3Len = 2;
    srcObj->m3 = (int16_t*)OsalMemCalloc(sizeof(int16_t) * srcObj->m3Len);
    for (uint32_t i = 0; i < srcObj->m3Len; i++) {
        srcObj->m3[i] = 3;
    }

    srcObj->m4Len = 2;
    srcObj->m4 = (int32_t*)OsalMemCalloc(sizeof(int32_t) * srcObj->m4Len);
    for (uint32_t i = 0; i < srcObj->m4Len; i++) {
        srcObj->m4[i] = 4;
    }

    srcObj->m5Len = 2;
    srcObj->m5 = (int64_t*)OsalMemCalloc(sizeof(int64_t) * srcObj->m5Len);
    for (uint32_t i = 0; i < srcObj->m5Len; i++) {
        srcObj->m5[i] = 5;
    }

    srcObj->m6Len = 2;
    srcObj->m6 = (uint8_t*)OsalMemCalloc(sizeof(uint8_t) * srcObj->m6Len);
    for (uint32_t i = 0; i < srcObj->m6Len; i++) {
        srcObj->m6[i] = 97;
    }

    srcObj->m7Len = 2;
    srcObj->m7 = (uint16_t*)OsalMemCalloc(sizeof(uint16_t) * srcObj->m7Len);
    for (uint32_t i = 0; i < srcObj->m7Len; i++) {
        srcObj->m7[i] = 7;
    }

    srcObj->m8Len = 2;
    srcObj->m8 = (uint32_t*)OsalMemCalloc(sizeof(uint32_t) * srcObj->m8Len);
    for (uint32_t i = 0; i < srcObj->m8Len; i++) {
        srcObj->m8[i] = 8;
    }

    srcObj->m9Len = 2;
    srcObj->m9 = (uint64_t*)OsalMemCalloc(sizeof(uint64_t) * srcObj->m9Len);
    for (uint32_t i = 0; i < srcObj->m9Len; i++) {
        srcObj->m9[i] = 9;
    }

    srcObj->m10Len = 2;
    srcObj->m10 = (float*)OsalMemCalloc(sizeof(float) * srcObj->m10Len);
    for (uint32_t i = 0; i < srcObj->m10Len; i++) {
        srcObj->m10[i] = 10.5;
    }

    srcObj->m11Len = 2;
    srcObj->m11 = (double*)OsalMemCalloc(sizeof(double) * srcObj->m11Len);
    for (uint32_t i = 0; i < srcObj->m11Len; i++) {
        srcObj->m11[i] = 11.55;
    }

    srcObj->m12Len = 2;
    srcObj->m12 = (char**)OsalMemCalloc(sizeof(char*) * srcObj->m12Len);
    for (uint32_t i = 0; i < srcObj->m12Len; i++) {
        const char* str = "hello world";
        srcObj->m12[i] = strdup(str);
    }

    srcObj->m13Len = 2;
    srcObj->m13 = (enum ESample*)OsalMemCalloc(sizeof(enum ESample) * srcObj->m13Len);
    for (uint32_t i = 0; i < srcObj->m13Len; i++) {
        srcObj->m13[i] = MEM_ONE;
    }

    srcObj->m14Len = 2;
    srcObj->m14 = (struct SSample*)OsalMemCalloc(sizeof(struct SSample) * srcObj->m14Len);
    for (uint32_t i = 0; i < srcObj->m14Len; i++) {
        srcObj->m14[i].m1 = true;
        srcObj->m14[i].m2 = 2;
        srcObj->m14[i].m3 = 3.55;
        srcObj->m14[i].m4 = strdup("hello");
    }

    struct SSample5* destObj = nullptr;
    int32_t ec = g_testClient->SSample5Test(g_testClient, srcObj, &destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);

    for (uint32_t i = 0; i < srcObj->m1Len; i++) {
        EXPECT_EQ((srcObj->m1[i] ? 1 : 0), (destObj->m1[i] ? 1 : 0));
    }

    for (uint32_t i = 0; i < srcObj->m2Len; i++) {
        EXPECT_EQ(srcObj->m2[i], destObj->m2[i]);
    }

    for (uint32_t i = 0; i < srcObj->m3Len; i++) {
        EXPECT_EQ(srcObj->m3[i], destObj->m3[i]);
    }

    for (uint32_t i = 0; i < srcObj->m4Len; i++) {
        EXPECT_EQ(srcObj->m4[i], destObj->m4[i]);
    }

    for (uint32_t i = 0; i < srcObj->m5Len; i++) {
        EXPECT_EQ(srcObj->m5[i], destObj->m5[i]);
    }

    for (uint32_t i = 0; i < srcObj->m6Len; i++) {
        EXPECT_EQ(srcObj->m6[i], destObj->m6[i]);
    }

    for (uint32_t i = 0; i < srcObj->m7Len; i++) {
        EXPECT_EQ(srcObj->m7[i], destObj->m7[i]);
    }

    for (uint32_t i = 0; i < srcObj->m8Len; i++) {
        EXPECT_EQ(srcObj->m8[i], destObj->m8[i]);
    }

    for (uint32_t i = 0; i < srcObj->m9Len; i++) {
        EXPECT_EQ(srcObj->m9[i], destObj->m9[i]);
    }

    for (uint32_t i = 0; i < srcObj->m10Len; i++) {
        EXPECT_FLOAT_EQ(srcObj->m10[i], destObj->m10[i]);
    }

    for (uint32_t i = 0; i < srcObj->m11Len; i++) {
        EXPECT_DOUBLE_EQ(srcObj->m11[i], destObj->m11[i]);
    }

    for (uint32_t i = 0; i < srcObj->m12Len; i++) {
        EXPECT_STREQ(srcObj->m12[i], destObj->m12[i]);
    }

    for (uint32_t i = 0; i < srcObj->m13Len; i++) {
        EXPECT_EQ(srcObj->m13[i], destObj->m13[i]);
    }

    for (uint32_t i = 0; i < srcObj->m14Len; i++) {
        EXPECT_EQ(((srcObj->m14[i]).m1 ? 1 : 0), ((destObj->m14[i]).m1 ? 1 : 0));
        EXPECT_EQ((srcObj->m14[i]).m2, (destObj->m14[i]).m2);
        EXPECT_DOUBLE_EQ((srcObj->m14[i]).m3, (destObj->m14[i]).m3);
        EXPECT_STREQ((srcObj->m14[i]).m4, (destObj->m14[i]).m4);
    }

    SSample5Free(srcObj, true);
    SSample5Free(destObj, true);
}

HWTEST_F(CStructTest, CStructTest_007, TestSize.Level1)
{
    struct SSample6* srcObj = (struct SSample6*)OsalMemCalloc(sizeof(struct SSample6));
    ASSERT_NE(srcObj, nullptr);

    srcObj->m1.m1 = true;
    srcObj->m1.m2 = 1;

    srcObj->m2Len = 2;
    srcObj->m2 = (union USample*)OsalMemCalloc(sizeof(union USample) * srcObj->m2Len);
    for (uint32_t i = 0; i < srcObj->m2Len; i++) {
        (srcObj->m2[i]).m1 = true;
        (srcObj->m2[i]).m2 = 2;
    }

    srcObj->m3Len = 2;
    srcObj->m3 = (union USample*)OsalMemCalloc(sizeof(union USample) * srcObj->m3Len);
    for (uint32_t i = 0; i < srcObj->m3Len; i++) {
        (srcObj->m3[i]).m1 = true;
        (srcObj->m3[i]).m2 = 2;
    }

    struct SSample6* destObj = nullptr;
    int32_t ec = g_testClient->SSample6Test(g_testClient, srcObj, &destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);

    EXPECT_EQ((srcObj->m1.m1 ? 1 : 0), (destObj->m1.m1 ? 1 : 0));
    EXPECT_EQ(srcObj->m1.m2, destObj->m1.m2);

    for (uint32_t i = 0; i < srcObj->m2Len; i++) {
        EXPECT_EQ(((srcObj->m2[i]).m1 ? 1 : 0), ((destObj->m2[i]).m1 ? 1 : 0));
        EXPECT_EQ((srcObj->m2[i]).m2, (destObj->m2[i]).m2);
    }

    for (uint32_t i = 0; i < srcObj->m3Len; i++) {
        EXPECT_EQ(((srcObj->m3[i]).m1 ? 1 : 0), ((destObj->m3[i]).m1 ? 1 : 0));
        EXPECT_EQ((srcObj->m3[i]).m2, (destObj->m3[i]).m2);
    }

    SSample6Free(srcObj, true);
    SSample6Free(destObj, true);
}

