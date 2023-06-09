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
#ifndef N_NAPI_PASTEDATA_RECORD_H
#define N_NAPI_PASTEDATA_RECORD_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "paste_data.h"
#include "paste_data_record.h"
#include "pasteboard_client.h"
#include "uri.h"

namespace OHOS {
namespace MiscServicesNapi {
class PasteDataRecordNapi {
public:
    static napi_value PasteDataRecordInit(napi_env env, napi_value exports);
    static napi_value New(napi_env env, napi_callback_info info);
    static napi_status NewInstance(napi_env env, napi_value &instance);
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);
    static bool NewInstanceByRecord(
        napi_env env, napi_value &instance, const std::shared_ptr<MiscServices::PasteDataRecord> &record);
    static bool NewHtmlTextRecordInstance(napi_env env, const std::string &text, napi_value &instance);
    static bool NewPlainTextRecordInstance(napi_env env, const std::string &text, napi_value &instance);
    static bool NewUriRecordInstance(napi_env env, const std::string &text, napi_value &instance);
    static bool NewWantRecordInstance(
        napi_env env, const std::shared_ptr<OHOS::AAFwk::Want> want, napi_value &instance);

    PasteDataRecordNapi();
    ~PasteDataRecordNapi();

    static napi_value ConvertToText(napi_env env, napi_callback_info info);

    std::shared_ptr<MiscServices::PasteDataRecord> value_;

private:
    bool JSFillInstance(napi_env env, napi_value &instance);
    void SetNamedPropertyByStr(napi_env env, napi_value &dstObj, const std::string &objName, const char *propName);
    napi_env env_;
    napi_ref wrapper_;
};
} // MiscServicesNapi
} // OHOS
#endif