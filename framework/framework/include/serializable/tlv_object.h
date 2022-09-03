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

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_OBJECT_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_OBJECT_H
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "api/visibility.h"
#include "endian_converter.h"

namespace OHOS::MiscServices {
#pragma pack(1)
struct TLVHead {
    uint16_t tag;
    uint32_t len;
    std::uint8_t value[0];
};
#pragma pack()
struct RawMem {
    uintptr_t buffer;
    size_t bufferLen;
};

/*
 * Common tag definitions.
 * Product should use after TAG_BUFF
 **/
enum COMMON_TAG : uint16_t {
    TAG_VECTOR = 0x0000,
    TAG_VECTOR_INFO,
    TAG_VECTOR_ITEM,

    TAG_BUFF = 0x0100,
};
struct API_EXPORT TLVObject {
public:
    virtual bool Encode(std::vector<std::uint8_t> &buffer) = 0;
    virtual bool Decode(const std::vector<std::uint8_t> &buffer) = 0;
    virtual size_t Count() = 0;

    static inline size_t Count(bool value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }
    static inline size_t Count(int8_t value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }
    static inline size_t Count(int16_t value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }
    static inline size_t Count(int32_t value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }
    static inline size_t Count(int64_t value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }
    static inline size_t Count(const std::string &value)
    {
        return value.size() + sizeof(TLVHead);
    }
    static inline size_t Count(const RawMem &value)
    {
        if (value.buffer == 0 || value.bufferLen == 0) {
            return 0;
        }
        return value.bufferLen + sizeof(TLVHead);
    }
    static inline size_t Count(TLVObject &value)
    {
        return value.Count() + sizeof(TLVHead);
    }
    template<typename T> inline size_t Count(std::shared_ptr<T> &value)
    {
        if (value == nullptr) {
            return 0;
        }
        return Count(*value);
    }
    template<typename T> inline size_t Count(std::vector<T> &value)
    {
        size_t expectSize = sizeof(TLVHead);
        for (auto &item : value) {
            expectSize += Count(item);
        }
        return expectSize;
    }

    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, bool value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, int8_t value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, int16_t value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, int32_t value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, int64_t value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, const std::string &value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, const RawMem &value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, TLVObject &value)
    {
        if (!HasExpectBuffer(buffer, sizeof(TLVHead))) {
            return false;
        }
        auto tagCursor = cursor_;
        cursor_ += sizeof(TLVHead);
        auto valueCursor = cursor_;
        bool ret = value.Encode(buffer, cursor_, buffer.size());
        WriteHead(buffer, type, tagCursor, cursor_ - valueCursor);
        return ret;
    }
    template<typename T> bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, std::vector<T> &value)
    {
        if (!HasExpectBuffer(buffer, sizeof(TLVHead))) {
            return false;
        }
        auto tagCursor = cursor_;
        cursor_ += sizeof(TLVHead);
        auto valueCursor = cursor_;
        bool ret = WriteValue(buffer, value);
        WriteHead(buffer, type, tagCursor, cursor_ - valueCursor);
        return ret;
    }
    template<typename T> bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, std::shared_ptr<T> &value)
    {
        if (value == nullptr) {
            return true;
        }
        return Write(buffer, type, *value);
    }
    bool ReadHead(const std::vector<std::uint8_t> &buffer, TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, bool &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, int8_t &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, int16_t &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, int32_t &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, int64_t &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, std::string &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, RawMem &rawMem, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, TLVObject &value, const TLVHead &head);
    template<typename T>
    bool ReadValue(const std::vector<std::uint8_t> &buffer, std::vector<T> &value, const TLVHead &head)
    {
        auto vectorEnd = cursor_ + head.len;
        for (; cursor_ < vectorEnd;) {
            // V: item value
            TLVHead valueHead{};
            bool ret = ReadHead(buffer, valueHead);
            T item{};
            ret = ret && ReadValue(buffer, item, valueHead);
            if (!ret) {
                return false;
            }
            value.push_back(item);
        }
        return true;
    }

    template<typename T>
    bool ReadValue(const std::vector<std::uint8_t> &buffer, std::shared_ptr<T> &value, const TLVHead &head)
    {
        value = std::make_shared<T>();
        if (value == nullptr) {
            return false;
        }
        return ReadValue(buffer, *value, head);
    }

protected:
    virtual ~TLVObject() = default;
    inline bool Skip(size_t len, size_t total)
    {
        if (total < len || total - len < cursor_) {
            return false;
        }
        cursor_ += len;
        return true;
    }
    inline bool IsEnough() const
    {
        return cursor_ < total_;
    }

    size_t total_ = 0;

private:
    bool Encode(std::vector<std::uint8_t> &buffer, size_t &cursor, size_t total);
    bool Decode(const std::vector<std::uint8_t> &buffer, size_t &cursor, size_t total);
    static inline void WriteHead(std::vector<std::uint8_t> &buffer, uint16_t type, size_t tagCursor, uint32_t len)
    {
        auto *tlvHead = reinterpret_cast<TLVHead *>(buffer.data() + tagCursor);
        tlvHead->tag = HostToNet(type);
        tlvHead->len = HostToNet(len);
    }
    static inline void WriteLen(std::vector<std::uint8_t> &buffer, size_t tagCursor, uint32_t len)
    {
        auto *pLen = reinterpret_cast<uint32_t *>(buffer.data() + tagCursor);
        *pLen = HostToNet(len);
    }
    template<typename T> bool WriteBasic(std::vector<std::uint8_t> &buffer, uint16_t type, T value)
    {
        if (!HasExpectBuffer(buffer, sizeof(TLVHead) + sizeof(value))) {
            return false;
        }
        auto *tlvHead = reinterpret_cast<TLVHead *>(buffer.data() + cursor_);
        tlvHead->tag = HostToNet(type);
        tlvHead->len = HostToNet((uint32_t)sizeof(value));
        *(reinterpret_cast<T *>(tlvHead->value)) = HostToNet(value);
        cursor_ += sizeof(TLVHead) + sizeof(value);
        return true;
    }

    template<typename T> bool WriteValue(std::vector<std::uint8_t> &buffer, std::vector<T> &value)
    {
        // items iterator
        bool ret = true;
        for (T &item : value) {
            // V:item value
            ret = ret && Write(buffer, TAG_VECTOR_ITEM, item);
        }
        return ret;
    }

    template<typename T> bool ReadBasicValue(const std::vector<std::uint8_t> &buffer, T &value, const TLVHead &head)
    {
        if (!HasExpectBuffer(buffer, head.len)) {
            return false;
        }
        value = NetToHost(*(reinterpret_cast<const T *>(buffer.data() + cursor_)));
        cursor_ += sizeof(T);
        return true;
    }

    inline uint32_t ReadLen(const std::vector<std::uint8_t> &buffer)
    {
        if (!HasExpectBuffer(buffer, sizeof(uint32_t))) {
            return false;
        }
        auto itemLen = *reinterpret_cast<const uint32_t *>(buffer.data() + cursor_);
        itemLen = NetToHost(itemLen);
        cursor_ += sizeof(uint32_t);
        return itemLen;
    }

    inline bool HasExpectBuffer(const std::vector<std::uint8_t> &buffer, uint32_t expectLen) const
    {
        return buffer.size() <= cursor_ || buffer.size() - cursor_ >= expectLen;
    }

    size_t cursor_ = 0;
};
} // namespace OHOS::MiscServices
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_OBJECT_H
