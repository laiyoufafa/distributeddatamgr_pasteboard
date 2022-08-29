/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "para_handle.h"

#include <cstring>
#include <thread>

#include "dev_profile.h"
#include "distributed_module_config.h"
#include "parameter.h"
#include "pasteboard_hilog_wreapper.h"
namespace OHOS {
namespace MiscServices {
constexpr int32_t HANDLE_OK = 0;
constexpr int32_t HANDLE_ERROR = -1;
const char *ParaHandle::DEFAULT_VALUE = "true";
const char *ParaHandle::DISTRIBUTED_PASTEBOARD_ENABLED = "persist.sys.distributedPasteboardEnabled";

ParaHandle::ParaHandle()
{
}

ParaHandle &ParaHandle::GetInstance()
{
    static ParaHandle instance;
    return instance;
}

void ParaHandle::ParameterChange(const char *key, const char *value, void *context)
{
    if (strncmp(key, DISTRIBUTED_PASTEBOARD_ENABLED, strlen(key)) != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "key is error.");
        return;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ParameterChange, key = %{public}s, value = %{public}s.", key, value);
    DevProfile::GetInstance().PutDeviceProfile(value);
    DistributedModuleConfig::Notify();
}

int32_t ParaHandle::Init(std::string &enabledStatus)
{
    char value[CONFIG_LEN] = { 0 };
    auto errNo = GetParameter(DISTRIBUTED_PASTEBOARD_ENABLED, "", value, CONFIG_LEN);
    if (errNo > HANDLE_OK) {
        enabledStatus = value;
    } else {
        errNo = SetParameter(DISTRIBUTED_PASTEBOARD_ENABLED, DEFAULT_VALUE);
        if (errNo != HANDLE_OK) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "SetParameter failed.");
            return errNo;
        }
        enabledStatus = DEFAULT_VALUE;
    }
    errNo = WatchParameter(DISTRIBUTED_PASTEBOARD_ENABLED, ParameterChange, nullptr);
    return errNo;
}

std::string ParaHandle::GetAndSubscribeEnabledStatus()
{
    std::string enabledStatus;
    auto errNo = Init(enabledStatus);
    if (errNo == HANDLE_OK) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetAndSubscribeEnabledStatus success.");
        return enabledStatus;
    }

    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetAndSubscribeEnabledStatus failed, try again");
    std::thread th = std::thread([&]() {
        constexpr int RETRY_TIMES = 300;
        int i = 0;
        int32_t errNo = HANDLE_ERROR;
        while (i++ < RETRY_TIMES) {
            errNo = Init(enabledStatus);
            if (errNo == HANDLE_OK) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        PASTEBOARD_HILOGI(
            PASTEBOARD_MODULE_SERVICE, "GetAndSubscribeEnabledStatus exit now: %{public}d times, errNo: %{public}d", i, errNo);
    });
    th.detach();
    return enabledStatus;
}

void ParaHandle::UnSubscribeEnabledStatus()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    WatchParameter(DISTRIBUTED_PASTEBOARD_ENABLED, nullptr, nullptr);
}
} // namespace MiscServices
} // namespace OHOS
