/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "pastedata_record_napi.h"
#include "pastedata_napi.h"
#include "pasteboard_common.h"
#include "napi_common.h"
#include "pasteboard_hilog_wreapper.h"
#include "paste_data_record.h"

using namespace OHOS::MiscServices;
using namespace OHOS::Media;

namespace OHOS {
namespace MiscServicesNapi {
static thread_local napi_ref g_pasteDataRecord = nullptr;
const size_t ARGC_TYPE_SET1 = 1;
const size_t CALLBACK_RESULT_NUM = 2;
constexpr int32_t  MIMETYPE_MAX_SIZE = 1024;

PasteDataRecordNapi::PasteDataRecordNapi() : env_(nullptr), wrapper_(nullptr)
{
}

PasteDataRecordNapi::~PasteDataRecordNapi()
{
    napi_delete_reference(env_, wrapper_);
}

bool PasteDataRecordNapi::NewInstanceByRecord(
    napi_env env, napi_value &instance, const std::shared_ptr<MiscServices::PasteDataRecord> &record)
{
    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = record;
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewHtmlTextRecordInstance(napi_env env, const std::string &text, napi_value &instance)
{
    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateHtmlTextRecord(text);
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewPlainTextRecordInstance(napi_env env, const std::string &text, napi_value &instance)
{
    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreatePlainTextRecord(text);
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewPixelMapRecordInstance(
    napi_env env, const std::shared_ptr<PixelMap> pixelMap, napi_value &instance)
{
    if (pixelMap == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "pixelMap is nullptr.");
        return false;
    }

    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "unwrap failed.");
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMap);
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewUriRecordInstance(napi_env env, const std::string &text, napi_value &instance)
{
    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateUriRecord(OHOS::Uri(text));
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewWantRecordInstance(
    napi_env env, const std::shared_ptr<OHOS::AAFwk::Want> want, napi_value &instance)
{
    if (!want) {
        return false;
    }

    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateWantRecord(want);
    obj->JSFillInstance(env, instance);
    return true;
}
bool PasteDataRecordNapi::NewKvRecordInstance(
    napi_env env, const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer, napi_value &instance)
{
    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "unwrap failed");
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    obj->JSFillInstance(env, instance);
    return true;
}
void PasteDataRecordNapi::SetNamedPropertyByStr(
    napi_env env, napi_value &instance, const char *propName, const char *propValue)
{
    if (propName == nullptr || propValue == nullptr) {
        return;
    }
    napi_value prop = nullptr;
    if (napi_create_string_utf8(env, propValue, NAPI_AUTO_LENGTH, &prop) == napi_ok) {
        napi_set_named_property(env, instance, propName, prop);
    }
}

napi_value PasteDataRecordNapi::SetNapiKvData(napi_env env, std::shared_ptr<MineCustomData> customData)
{
    if (customData == nullptr) {
        return nullptr;
    }
    napi_value jsCustomData = nullptr;
    napi_create_object(env, &jsCustomData);
    auto itemData = customData->GetItemData();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "size = %{public}d.", static_cast<uint32_t>(itemData.size()));
    for (auto &item : itemData) {
        void *data = nullptr;
        napi_value arrayBuffer = nullptr;
        size_t len = item.second.size();
        NAPI_CALL(env, napi_create_arraybuffer(env, len, &data, &arrayBuffer));
        if (memcpy_s(data, len, reinterpret_cast<const void *>(item.second.data()), len) != 0) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_named_property(env, jsCustomData, item.first.c_str(), arrayBuffer));
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "mimeType = %{public}s.", item.first.c_str());
    }
    return jsCustomData;
}

std::shared_ptr<MineCustomData> PasteDataRecordNapi::GetNativeKvData(napi_env env, napi_value napiValue)
{
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, napiValue, &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    napi_value mimeTypes = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, napiValue, &mimeTypes));
    uint32_t mimeTypesNum = 0;
    NAPI_CALL(env, napi_get_array_length(env, mimeTypes, &mimeTypesNum));

    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "mimeTypesNum = %{public}d", mimeTypesNum);
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();
    for (uint32_t i = 0; i < mimeTypesNum; i++) {
        napi_value mimeTypeNapi = nullptr;
        NAPI_CALL(env, napi_get_element(env, mimeTypes, i, &mimeTypeNapi));

        std::string mimeType;
        if (!(MiscServicesNapi::GetValue(env, mimeTypeNapi, mimeType)) || (mimeType.size() > MIMETYPE_MAX_SIZE)) {
            return nullptr;
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "mimeType = %{public}s,", mimeType.c_str());

        napi_value napiArrayBuffer = nullptr;
        NAPI_CALL(env, napi_get_property(env, napiValue, mimeTypeNapi, &napiArrayBuffer));
        void *data = nullptr;
        size_t dataLen;
        NAPI_CALL(env, napi_get_arraybuffer_info(env, napiArrayBuffer, &data, &dataLen));
        customData->AddItemData(mimeType,
            std::vector<uint8_t>(reinterpret_cast<uint8_t *>(data), reinterpret_cast<uint8_t *>(data) + dataLen));
    }
    return customData;
}

void PasteDataRecordNapi::JSFillInstance(napi_env env, napi_value &instance)
{
    if (value_ == nullptr) {
        return;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "mimeType = %{public}s.", value_->GetMimeType().c_str());

    auto mimeType = value_->GetMimeType();
    SetNamedPropertyByStr(env, instance, "mimeType", mimeType.c_str());

    auto plainText = value_->GetPlainText();
    if (plainText != nullptr) {
        SetNamedPropertyByStr(env, instance, "plainText", plainText->c_str());
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill plainText.");
    }

    auto htmlText = value_->GetHtmlText();
    if (htmlText != nullptr) {
        SetNamedPropertyByStr(env, instance, "htmlText", htmlText->c_str());
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill htmlText.");
    }

    auto uri = value_->GetUri();
    if (uri != nullptr) {
        SetNamedPropertyByStr(env, instance, "uri", uri->ToString().c_str());
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill uri.");
    }

    auto want = value_->GetWant();
    if (want != nullptr) {
        napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, *want);
        napi_set_named_property(env, instance, "want", jsWant);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill want.");
    }

    auto pixelMap = value_->GetPixelMap();
    if (pixelMap != nullptr) {
        napi_value jsPixelMap = OHOS::Media::PixelMapNapi::CreatePixelMap(env, pixelMap);
        napi_set_named_property(env, instance, "pixelMap", jsPixelMap);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill pixelMap.");
    }

    auto customData = value_->GetCustomData();
    if (customData != nullptr) {
        napi_value jsCustomData = PasteDataRecordNapi::SetNapiKvData(env, customData);
        napi_set_named_property(env, instance, "data", jsCustomData);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill data.");
    }
}

using AsyncText = struct AsyncText {
    napi_env env = nullptr;
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    PasteDataRecordNapi *obj = nullptr;
    int32_t status = 0;
    std::string text;
};

void AsyncCompleteCallbackConvertToText(napi_env env, napi_status status, void *data)
{
    if (!data) {
        return;
    }
    AsyncText *asyncText = (AsyncText *)data;
    napi_value results[CALLBACK_RESULT_NUM] = { 0 };
    if (asyncText->status == 0) {
        napi_get_undefined(env, &results[0]);
        napi_create_string_utf8(env, asyncText->text.c_str(), NAPI_AUTO_LENGTH, &results[1]);
    } else {
        napi_value message = nullptr;
        napi_create_string_utf8(env, "ConvertToText Failed", NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &results[0]);
        napi_get_undefined(env, &results[1]);
    }
    if (asyncText->deferred != nullptr) {
        if (asyncText->status == 0) {
            napi_resolve_deferred(env, asyncText->deferred, results[1]);
        } else {
            napi_reject_deferred(env, asyncText->deferred, results[0]);
        }
    } else {
        napi_value callback = nullptr;
        napi_value resultOut = nullptr;
        napi_get_reference_value(env, asyncText->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, CALLBACK_RESULT_NUM, results, &resultOut);
        napi_delete_reference(env, asyncText->callbackRef);
    }
    napi_delete_async_work(env, asyncText->work);
    delete asyncText;
    asyncText = nullptr;
}

napi_value PasteDataRecordNapi::ConvertToText(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
   
    PasteDataRecordNapi *obj = nullptr;
    auto status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get ConvertToText object failed");
        return nullptr;
    }
    AsyncText *asyncText = new (std::nothrow) AsyncText {.env = env, .work = nullptr, .obj = obj};
    if (asyncText == nullptr) {
        return NapiGetNull(env);
    }
    if (argc >= ARGC_TYPE_SET1) {
        napi_valuetype valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &asyncText->callbackRef);
    }
    napi_value promise = nullptr;
    if (asyncText->callbackRef == nullptr) {
        napi_create_promise(env, &asyncText->deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "ConvertToText", NAPI_AUTO_LENGTH, &resource);
    napi_status syncWork = napi_create_async_work(env,
        nullptr, resource,
        [](napi_env env, void *data) {
            AsyncText *asyncText = (AsyncText *)data;
            if (asyncText->obj == nullptr || asyncText->obj->value_ == nullptr) {
                asyncText->status = -1;
            } else {
                asyncText->text = asyncText->obj->value_->ConvertToText();
                asyncText->status = 0;
            }
        },
        AsyncCompleteCallbackConvertToText,
        (void *)asyncText, &asyncText->work);
    napi_queue_async_work(env, asyncText->work);
    if (syncWork != napi_ok) {
        delete asyncText;
        asyncText = nullptr;
    }

    return promise;
}

napi_value PasteDataRecordNapi::PasteDataRecordInit(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("convertToText", ConvertToText),
    };

    napi_status status = napi_ok;

    napi_value constructor;
    status = napi_define_class(env,
        "PasteDataRecord",
        NAPI_AUTO_LENGTH,
        New,
        nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor),
        properties,
        &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to define class at PasteDataRecordInit");
        return nullptr;
    }

    status = napi_create_reference(env, constructor, 1, &g_pasteDataRecord);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "PasteDataRecordInit create referece failed");
        return nullptr;
    }
    napi_set_named_property(env, exports, "PasteDataRecord", constructor);
    return exports;
}

void PasteDataRecordNapi::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    PasteDataRecordNapi *obj = static_cast<PasteDataRecordNapi *>(nativeObject);
    delete obj;
}

napi_value PasteDataRecordNapi::New(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    // get native object
    PasteDataRecordNapi *obj = new PasteDataRecordNapi();
    obj->env_ = env;
    NAPI_CALL(env, napi_wrap(env, thisVar, obj, PasteDataRecordNapi::Destructor, nullptr, &obj->wrapper_));
    return thisVar;
}

napi_status PasteDataRecordNapi::NewInstance(napi_env env, napi_value &instance)
{
    napi_status status = napi_invalid_arg;

    napi_value constructor;
    status = napi_get_reference_value(env, g_pasteDataRecord, &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "get referece failed");
        return status;
    }

    status = napi_new_instance(env, constructor, 0, nullptr, &instance);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "new instance failed");
        return status;
    }

    return napi_ok;
}
} // namespace MiscServicesNapi
} // namespace OHOS
