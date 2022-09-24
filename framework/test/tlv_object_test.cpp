/*
* Copyright (c) 2022 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "tlv_object.h"

#include <gtest/gtest.h>

#include "pasteboard_client.h"
#include "pasteboard_hilog_wreapper.h"
namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace OHOS::AAFwk;

enum TAG_VALUE : uint16_t {
    TAG_BOOL = 1,
    TAG_INT8,
    TAG_INT16,
};

class TLVObjectTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::shared_ptr<PasteDataRecord> GenRecord(std::uint32_t index);
};

void TLVObjectTest::SetUpTestCase(void)
{
}

void TLVObjectTest::TearDownTestCase(void)
{
}

void TLVObjectTest::SetUp(void)
{
}

void TLVObjectTest::TearDown(void)
{
}

struct TLVTest : public TLVObject {
public:
    TLVTest() = default;
    TLVTest(bool valueBool, int8_t valueInt8, int16_t valueInt16);
    virtual bool Encode(std::vector<std::uint8_t> &buffer);
    virtual bool Decode(const std::vector<std::uint8_t> &buffer);
    virtual size_t Count();
    bool GetBool();
    int8_t GetInt8();
    int16_t GetInt16();

private:
    bool valueBool_;
    int8_t valueInt8_;
    int16_t valueInt16_;
};

TLVTest::TLVTest(bool valueBool, int8_t valueInt8, int16_t valueInt16)
    : valueBool_{ valueBool }, valueInt8_{ valueInt8 }, valueInt16_{ valueInt16 }
{
}

bool TLVTest::Encode(std::vector<std::uint8_t> &buffer)
{
    Init(buffer);
    bool ret = Write(buffer, TAG_BOOL, valueBool_);
    ret = Write(buffer, TAG_INT8, valueInt8_) && ret;
    ret = Write(buffer, TAG_INT16, valueInt16_) && ret;
    return ret;
}

bool TLVTest::Decode(const std::vector<std::uint8_t> &buffer)
{
    total_ = buffer.size();
    for (; IsEnough();) {
        TLVHead head{};
        bool ret = ReadHead(buffer, head);
        switch (head.tag) {
            case TAG_BOOL:
                ret = ret && ReadValue(buffer, valueBool_, head);
                break;
            case TAG_INT8: {
                ret = ret && ReadValue(buffer, valueInt8_, head);
                break;
            }
            case TAG_INT16: {
                ret = ret && ReadValue(buffer, valueInt16_, head);
                break;
            }
            default:
                ret = ret && Skip(head.len, buffer.size());
                break;
        }
        if (!ret) {
            return false;
        }
    }
    return true;
}

size_t TLVTest::Count()
{
    size_t expectSize = 0;
    expectSize += sizeof(TLVHead);
    expectSize += TLVObject::Count(valueBool_);
    expectSize += TLVObject::Count(valueInt8_);
    expectSize += TLVObject::Count(valueInt16_);
    return expectSize;
}

bool TLVTest::GetBool()
{
    return valueBool_;
}
int8_t TLVTest::GetInt8()
{
    return valueInt8_;
}
int16_t TLVTest::GetInt16()
{
    return valueInt16_;
}

std::shared_ptr<PasteDataRecord> TLVObjectTest::GenRecord(std::uint32_t index)
{
    std::string indexStr = "";
    auto plainText = std::make_shared<std::string>("hello" + indexStr);
    auto htmlText = std::make_shared<std::string>("<span>hello" + indexStr + "</span>");
    auto uri = std::make_shared<OHOS::Uri>("dataability://hello" + indexStr + ".txt");
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    want->SetParam(key, id);

    PasteDataRecord::Builder builder(MIMETYPE_TEXT_HTML);
    auto record1 = builder.SetPlainText(plainText).SetHtmlText(htmlText).SetUri(uri).SetWant(want).Build();
    return record1;
}

/**
* @tc.name: TLVOjbectTest001
* @tc.desc: test tlv coder.
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: baoyayong
*/
HWTEST_F(TLVObjectTest, TLVOjbectTest001, TestSize.Level0)
{
    PasteData pasteData1;
    for (uint32_t i = 0; i < 10; ++i) {
        pasteData1.AddRecord(TLVObjectTest::GenRecord(i));
    }

    std::vector<uint8_t> buffer;
    auto ret = pasteData1.Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), pasteData1.Count());

    PasteData pasteData2;
    ret = pasteData2.Decode(buffer);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1.GetRecordCount());

    for (uint32_t i = 0; i < 10; ++i) {
        auto record2 = pasteData2.GetRecordAt(i);
        auto record1 = pasteData1.GetRecordAt(i);
        EXPECT_EQ(*(record2->GetHtmlText()), *(record1->GetHtmlText()));
        EXPECT_EQ(*(record2->GetPlainText()), *(record1->GetPlainText()));
        EXPECT_TRUE(record2->GetUri()->ToString() == record1->GetUri()->ToString());
        EXPECT_EQ(record2->GetWant()->OperationEquals(*(record1->GetWant())), true);
    }
}

/**
* @tc.name: TLVOjbectTest002
* @tc.desc: test tlv coder.
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: baoyayong
*/
HWTEST_F(TLVObjectTest, TLVOjbectTest002, TestSize.Level0)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    auto pasteData1 = PasteboardClient::GetInstance()->CreateWantData(std::make_shared<Want>(wantIn));

    std::vector<uint8_t> buffer;
    auto ret = pasteData1->Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), pasteData1->Count());

    PasteData pasteData2;
    ret = pasteData2.Decode(buffer);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1->GetRecordCount());

    auto record2 = pasteData2.GetRecordAt(0);
    auto record1 = pasteData1->GetRecordAt(0);
    EXPECT_EQ(record2->GetWant()->OperationEquals(*(record1->GetWant())), true);
}

/**
* @tc.name: TLVOjbectTest003
* @tc.desc: test tlv coder map.
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: baoyayong
*/
HWTEST_F(TLVObjectTest, TLVOjbectTest003, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData1 = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType1 = "image/jpg";
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();
    customData->AddItemData(mimeType1, arrayBuffer);
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_PLAIN);
    std::shared_ptr<PasteDataRecord> pasteDataRecord = builder.SetCustomData(customData).Build();
    pasteData1->AddRecord(pasteDataRecord);

    std::vector<uint8_t> buffer;
    auto ret = pasteData1->Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), pasteData1->Count());

    PasteData pasteData2;
    ret = pasteData2.Decode(buffer);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1->GetRecordCount());
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1->GetRecordCount());

    auto record1 = pasteData1->GetRecordAt(0);
    auto record2 = pasteData2.GetRecordAt(0);
    ASSERT_TRUE(record2 != nullptr);
    auto custom2 = record2->GetCustomData();
    auto custom1 = record1->GetCustomData();
    ASSERT_TRUE(custom1 != nullptr && custom2 != nullptr);
    EXPECT_EQ(custom2->GetItemData().size(), custom1->GetItemData().size());
}

/**
* @tc.name: WriteReadValueTest001
* @tc.desc: Write、ReadValue.
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: chenyu
*/
HWTEST_F(TLVObjectTest, WriteReadValueTest001, TestSize.Level0)
{
    TLVTest tlvTest(false, -1, 8);
    std::vector<uint8_t> buffer;
    auto ret = tlvTest.Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), tlvTest.Count());

    TLVTest tlvTest1;
    ret = tlvTest1.Decode(buffer);
    ASSERT_TRUE(ret);

    EXPECT_EQ(tlvTest1.Count(), tlvTest.Count());
    EXPECT_EQ(tlvTest1.GetBool(), tlvTest.GetBool());
    EXPECT_EQ(tlvTest1.GetInt8(), tlvTest.GetInt8());
    EXPECT_EQ(tlvTest1.GetInt16(), tlvTest.GetInt16());
}
} // namespace OHOS::MiscServices