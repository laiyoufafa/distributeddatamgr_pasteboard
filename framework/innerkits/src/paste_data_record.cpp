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
#include "paste_data_record.h"
#include "pasteboard_common.h"

namespace OHOS {
namespace MiscServices {
namespace {
constexpr int MAX_TEXT_LEN = 500 * 1024;
}
std::shared_ptr<PasteDataRecord> PasteDataRecord::NewHtmlRecord(const std::string &htmlText)
{
    if (htmlText.length() >= MAX_TEXT_LEN) {
        return nullptr;
    }
    return std::make_shared<PasteDataRecord>(MIMETYPE_TEXT_HTML,
                                             std::make_shared<std::string>(htmlText),
                                             nullptr,
                                             nullptr,
                                             nullptr);
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    return std::make_shared<PasteDataRecord>(MIMETYPE_TEXT_WANT,
                                           nullptr,
                                           std::move(want),
                                           nullptr,
                                           nullptr);
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewPlaintTextRecord(const std::string &text)
{
    if (text.length() >= MAX_TEXT_LEN) {
        return nullptr;
    }
    return std::make_shared<PasteDataRecord>(MIMETYPE_TEXT_PLAIN,
                                             nullptr,
                                             nullptr,
                                             std::make_shared<std::string>(text),
                                             nullptr);
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewUriRecord(const OHOS::Uri &uri)
{
    return std::make_shared<PasteDataRecord>(MIMETYPE_TEXT_URI,
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             std::make_shared<OHOS::Uri>(uri));
}

PasteDataRecord::PasteDataRecord(std::string mimeType,
                                 std::shared_ptr<std::string> htmlText,
                                 std::shared_ptr<OHOS::AAFwk::Want> want,
                                 std::shared_ptr<std::string> plainText,
                                 std::shared_ptr<OHOS::Uri> uri)
    : mimeType_ {std::move(mimeType)},
      htmlText_ {std::move(htmlText)},
      want_ {std::move(want)},
      plainText_ {std::move(plainText)},
      uri_ {std::move(uri)} {}

std::shared_ptr<std::string> PasteDataRecord::GetHtmlText() const
{
    return this->htmlText_;
}

std::string PasteDataRecord::GetMimeType() const
{
    return this->mimeType_;
}

std::shared_ptr<std::string> PasteDataRecord::GetPlainText() const
{
    return this->plainText_;
}

std::shared_ptr<OHOS::Uri> PasteDataRecord::GetUri() const
{
    return this->uri_;
}

std::shared_ptr<OHOS::AAFwk::Want> PasteDataRecord::GetWant() const
{
    return this->want_;
}
std::string PasteDataRecord::ConvertToText() const
{
    if (this->htmlText_) {
        return *this->htmlText_;
    } else if (this->plainText_) {
        return *this->plainText_;
    } else if (this->uri_) {
        return this->uri_->ToString();
    } else {
        return "";
    }
}

bool PasteDataRecord::Marshalling(Parcel &parcel) const
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start: %{public}s,", mimeType_.c_str());
    // write mimeType_
    if (!parcel.WriteString16(Str8ToStr16(mimeType_))) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
        return false;
    }

    if (mimeType_ == MIMETYPE_TEXT_PLAIN) {
        // write plainText_
        if (plainText_ != nullptr) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "plainText_: %{public}s,", (*plainText_).c_str());
            if (!parcel.WriteString16(Str8ToStr16(*plainText_))) {
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "write failed.");
                return false;
            }
        }
    } else if (mimeType_ == MIMETYPE_TEXT_HTML) {
        if (htmlText_ != nullptr) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "htmlText_: %{public}s,", (*htmlText_).c_str());
            if (!parcel.WriteString16(Str8ToStr16(*htmlText_))) {
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "write failed.");
                return false;
            }
        }
    } else if (mimeType_ == MIMETYPE_TEXT_URI) {
        // write uri_
        if (uri_ != nullptr) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "uri_: %{public}s,", uri_->ToString().c_str());
            if (!parcel.WriteParcelable(uri_.get())) {
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "write failed.");
                return false;
            }
        }
    } else if (mimeType_ == MIMETYPE_TEXT_WANT) {
        // write want
        if (want_ != nullptr) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Write want_");
            if (!parcel.WriteParcelable(want_.get())) {
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "write failed.");
                return false;
            }
        }
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Unkonw MimeType: %{public}s.", mimeType_.c_str());
        return false;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return true;
}

bool PasteDataRecord::ReadFromParcel(Parcel &parcel)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    // read mimeType_
    mimeType_ = Str16ToStr8(parcel.ReadString16());
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "mimeType_: %{public}s,", mimeType_.c_str());

    if (mimeType_ == MIMETYPE_TEXT_HTML) {
        // read htmlText_
        htmlText_ = std::make_shared<std::string>(Str16ToStr8(parcel.ReadString16()));
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "htmlText_: %{public}s,", (*htmlText_).c_str());
    } else if (mimeType_ == MIMETYPE_TEXT_PLAIN) {
        // read plainText_
        plainText_ = std::make_shared<std::string>(Str16ToStr8(parcel.ReadString16()));
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "plainText_: %{public}s,", (*plainText_).c_str());
    } else if (mimeType_ == MIMETYPE_TEXT_URI) {
        // read uri_
        std::unique_ptr<OHOS::Uri> uri(parcel.ReadParcelable<OHOS::Uri>());
        if (!uri) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "nullptr.");
            return false;
        }
        uri_ = std::make_shared<OHOS::Uri>(*uri);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "uri_: %{public}s,", uri_->ToString().c_str());
    } else if (mimeType_ == MIMETYPE_TEXT_WANT) {
        // read want_
        std::unique_ptr<OHOS::AAFwk::Want> want(parcel.ReadParcelable<OHOS::AAFwk::Want>());
        if (!want) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "nullptr.");
            return false;
        }
        want_ = std::make_shared<OHOS::AAFwk::Want>(*want);
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Unkonw MimeType: %{public}s.", mimeType_.c_str());
        return false;
    }

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return true;
}

PasteDataRecord *PasteDataRecord::Unmarshalling(Parcel &parcel)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    PasteDataRecord *pasteDataRecord = new PasteDataRecord();

    if (pasteDataRecord && !pasteDataRecord->ReadFromParcel(parcel)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "delete end.");
        delete pasteDataRecord;
        pasteDataRecord = nullptr;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return pasteDataRecord;
}
} // MiscServices
} // OHOS