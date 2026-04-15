#ifndef DATAMODEL_DESIGN_STRINGMGR_H_
#define DATAMODEL_DESIGN_STRINGMGR_H_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include "Utils/Unils.h"

namespace db {

template <typename IdType, typename ValueType>
class TableBase {
public:
    virtual ~TableBase()                      = default;
    virtual IdType    Add(const ValueType& v) = 0;
    virtual ValueType Get(IdType id) const    = 0;
    virtual void      Destroy(IdType id)      = 0;
    virtual void      Clear()                 = 0;
};

template <typename IdType>
class StringTableBase {
public:
    virtual ~StringTableBase()                                 = default;
    virtual IdType           AddString(const std::string& str) = 0;
    virtual std::string_view GetString(IdType id) const        = 0;
    virtual void             DestroyString(IdType id)          = 0;
    virtual void             Clear()                           = 0;
};

// ==================== StringMgr 主类 ====================
template <typename IdType, uint8_t MAX_FIX_LENGTH = 127, IdType MinElementCount = 128,
          uint16_t BlockSizeMultiple = 1>
class StringMgr {
    static_assert(std::is_integral<IdType>::value, "IdType must be integral");
    static_assert(MAX_FIX_LENGTH > 0 && MAX_FIX_LENGTH <= 127,
                  "MAX_FIX_LENGTH must be between 1 and 127");
    static_assert(MinElementCount > 0, "MinElementCount must be greater than 0");
    static_assert(BlockSizeMultiple > 0, "BlockSizeMultiple must be greater than 0");

    // ---------- 编译期常量 ----------
    static constexpr size_t LCM_VALUE      = LCMRange<MAX_FIX_LENGTH>::value;
    static constexpr size_t MAX_INLINE_LEN = sizeof(IdType);
    template <size_t Len>
    static constexpr size_t FindBestK() {
        constexpr size_t target  = MinElementCount;
        size_t           larger  = NextPerfectSize(target);
        size_t           smaller = (larger + 1) / 2 - 1;
        if (smaller == 0) smaller = 1;

        auto calc_k = [](size_t elements, size_t len) -> size_t {
            return (elements * len + LCM_VALUE - 1) / LCM_VALUE;
        };

        size_t k_large = calc_k(larger, Len);
        size_t k_small = calc_k(smaller, Len);

        size_t actual_large = LCM_VALUE * k_large / Len;
        size_t actual_small = LCM_VALUE * k_small / Len;

        if (IsPerfectSize(actual_large)) return k_large;
        if (IsPerfectSize(actual_small)) return k_small;

        return (std::abs((int64_t)actual_large - (int64_t)larger) <=
                std::abs((int64_t)actual_small - (int64_t)smaller))
                   ? k_large
                   : k_small;
    }

    template <size_t... Is>
    static constexpr size_t ComputeOptimalK(std::index_sequence<Is...>) {
        return std::max({FindBestK<Is + 1>()...});
    }

    static constexpr size_t OPTIMAL_K = ComputeOptimalK(std::make_index_sequence<MAX_FIX_LENGTH>());

    static constexpr size_t BASE_BLOCK_BYTES = LCM_VALUE * OPTIMAL_K;
    static constexpr size_t BLOCK_BYTES      = BASE_BLOCK_BYTES * BlockSizeMultiple;

    static constexpr size_t TOTAL_BITS     = sizeof(IdType) * 8;
    static constexpr size_t FIXED_LEN_BITS = 7;
    static constexpr size_t FLAG_BIT       = 1;

    // ---------- 定长表参数（每个长度独立） ----------
    template <size_t Len>
    struct FixedTableParams {
        static constexpr size_t elements_per_block = BLOCK_BYTES / Len;
        static constexpr size_t offset_bits        = []() {
            size_t bits = 0;
            size_t temp = elements_per_block;
            while (temp > 0) {
                ++bits;
                temp >>= 1;
            }
            return bits;
        }();
        static constexpr size_t available_bits = TOTAL_BITS - FLAG_BIT - FIXED_LEN_BITS;
        static constexpr size_t block_bits     = available_bits - offset_bits;
        static_assert(block_bits > 0, "IdType too small to encode block index");

        static constexpr IdType offset_mask = (static_cast<IdType>(1) << offset_bits) - 1;
        static constexpr IdType block_mask  = (static_cast<IdType>(1) << block_bits) - 1;

        static constexpr IdType Encode(size_t block_idx, size_t offset) {
            IdType id = 0;
            id |= (static_cast<IdType>(Len) << 1);
            id |= (static_cast<IdType>(block_idx) << (offset_bits + 1 + FIXED_LEN_BITS));
            id |= (static_cast<IdType>(offset) << (1 + FIXED_LEN_BITS));
            return id;
        }

        static constexpr std::pair<size_t, size_t> Decode(IdType id) {
            size_t offset    = (id >> (1 + FIXED_LEN_BITS)) & offset_mask;
            size_t block_idx = (id >> (offset_bits + 1 + FIXED_LEN_BITS)) & block_mask;
            return {block_idx, offset};
        }
    };

    // ---------- 定长表实现 ----------
    template <size_t Len>
    class FixLengthStringTable : public StringTableBase<IdType> {
    public:
        static constexpr auto PARAMS = FixedTableParams<Len>();

        FixLengthStringTable() {
            m_blocks_.reserve(16);
            m_free_list_.reserve(64);
        }

        ~FixLengthStringTable() override { Clear(); }

        IdType AddString(const std::string& str) override {
            assert(str.length() == Len);
            // 1. 优先使用空闲槽位
            if (!m_free_list_.empty()) {
                auto [block_idx, offset] = m_free_list_.back();
                m_free_list_.pop_back();
                char* block_data = m_blocks_[block_idx];
                std::memcpy(block_data + offset * Len, str.data(), Len);
                return PARAMS.Encode(block_idx, offset);
            }
            if (m_blocks_.empty() || m_cur_count_ >= PARAMS.elements_per_block) {
                m_cur_block_     = m_blocks_.size();
                char* block_data = (char*)malloc(BLOCK_BYTES);
                if (block_data == nullptr) {
                    throw std::runtime_error("Failed to allocate block");
                }
                m_blocks_.emplace_back(block_data);
                m_cur_count_ = 0;
            }
            char* block_data = m_blocks_[m_cur_block_];
            std::memcpy(block_data + m_cur_count_ * Len, str.data(), Len);
            auto id = PARAMS.Encode(m_cur_block_, m_cur_count_);
            m_cur_count_ += 1;
            return id;
        }

        std::string_view GetString(IdType id) const override {
            auto [block_idx, offset] = PARAMS.Decode(id);
            const char* block_data   = m_blocks_[block_idx];
            return std::string_view(block_data + offset * Len, Len);
        }

        void DestroyString(IdType id) override {
            auto [block_idx, offset] = PARAMS.Decode(id);
            if (block_idx == m_cur_block_ && offset == m_cur_count_ - 1) {
                // 直接回收当前块末尾的字符串，无需加入空闲列表
                m_cur_count_ -= 1;
                return;
            }
            m_free_list_.emplace_back(block_idx, offset);
        }

        void Clear() override {
            for (auto& block : m_blocks_) {
                free(block);
            }
            m_blocks_.clear();
            m_free_list_.clear();
            m_cur_block_ = 0;
            m_cur_count_ = 0;
        }

    private:
        std::vector<char*>                     m_blocks_;
        size_t                                 m_cur_block_ = 0;
        size_t                                 m_cur_count_ = 0;
        std::vector<std::pair<IdType, IdType>> m_free_list_;
    };

    // ---------- 变长表实现（按长度精确匹配空闲） ----------
    class DynamicStringTable : public StringTableBase<IdType> {
    public:
        static constexpr size_t OFFSET_BITS = 20;
        static constexpr size_t BLOCK_BITS  = TOTAL_BITS - 1 - OFFSET_BITS;
        static_assert(BLOCK_BITS > 0, "IdType too small for dynamic table");

        DynamicStringTable() { m_blocks_.reserve(16); }

        ~DynamicStringTable() override { Clear(); }

        IdType AddString(const std::string& str) override {
            uint16_t len         = static_cast<uint16_t>(str.length());
            size_t   total_bytes = len + 2;

            if (total_bytes > BLOCK_BYTES) {
                throw std::runtime_error("String too long for dynamic block");
            }

            // 1. 尝试复用相同长度的空闲槽位
            auto it = m_free_by_len_.find(len);
            if (it != m_free_by_len_.end() && !it->second.empty()) {
                IdType id = it->second.back();
                it->second.pop_back();
                if (it->second.empty()) {
                    m_free_by_len_.erase(it);
                }
                auto [block_idx, offset] = Decode(id);
                char* block_data         = m_blocks_[block_idx];
                // 长度不需要更新，因为是相同长度的空闲槽位
                std::memcpy(block_data + offset + 2, str.data(), len);
                return id;
            } else {
                // +MAX_FIX_LENGTH因为定长表最大长度是MAX_FIX_LENGTH，如果当前字符串长度超过MAX_FIX_LENGTH，则不可能有定长表的空闲槽位可以复用，所以直接跳过定
                // +3是因为动态表的每个槽位至少需要2字节存储长度信息，如果剩余空间不足2字节，则无法形成一个新的空闲槽位
                auto upper_len_it = total_bytes + MAX_FIX_LENGTH + 3;
                auto upper_it     = m_free_by_len_.upper_bound(upper_len_it);
                if (upper_it != m_free_by_len_.end() && !upper_it->second.empty()) {
                    IdType id = upper_it->second.back();
                    upper_it->second.pop_back();
                    if (upper_it->second.empty()) {
                        m_free_by_len_.erase(upper_it);
                    }
                    auto [block_idx, offset] = Decode(id);
                    char* block_data         = m_blocks_[block_idx];
                    // 更新长度
                    std::memcpy(block_data + offset, &len, 2);
                    std::memcpy(block_data + offset + 2, str.data(), len);
                    // 将剩余空间重新加入空闲列表
                    uint16_t free_len = static_cast<uint16_t>(upper_it->first - total_bytes);
                    // 不用判断长度是否大于2，因为upper_it->first至少是total_bytes
                    // + MAX_FIX_LENGTH + 3
                    std::memcpy(block_data + offset + total_bytes, &free_len, 2);
                    IdType free_id = Encode(block_idx, offset + total_bytes);
                    m_free_by_len_[free_len].push_back(free_id);
                    return id;
                }
            }

            // 初始化为0，避免额外判断m_blocks_是否为空
            if (m_cur_remainder_ < total_bytes) {
                if (m_cur_remainder_ > 2 + MAX_FIX_LENGTH) {
                    // 2. 当前块剩余空间不足，且可以形成一个新的空闲槽位
                    char*    block_data = m_blocks_[m_cur_block_];
                    uint16_t free_len   = static_cast<uint16_t>(m_cur_remainder_ - 2);
                    std::memcpy(block_data + BLOCK_BYTES - m_cur_remainder_, &free_len, 2);
                    IdType free_id = Encode(m_cur_block_, BLOCK_BYTES - m_cur_remainder_);
                    m_free_by_len_[free_len].push_back(free_id);
                }
                m_cur_block_     = m_blocks_.size();
                char* block_data = (char*)malloc(BLOCK_BYTES);
                if (block_data == nullptr) {
                    throw std::runtime_error("Failed to allocate block");
                }
                m_blocks_.emplace_back(block_data);
                m_cur_remainder_ = BLOCK_BYTES;
            }

            char* block_data = m_blocks_[m_cur_block_];
            std::memcpy(block_data, &len, 2);
            std::memcpy(block_data + 2, str.data(), len);
            auto id = Encode(m_cur_block_, BLOCK_BYTES - m_cur_remainder_);
            m_cur_remainder_ -= total_bytes;
            return id;
        }

        std::string_view GetString(IdType id) const override {
            auto [block_idx, offset] = Decode(id);
            const char* block_data   = m_blocks_[block_idx];
            uint16_t    len          = *reinterpret_cast<const uint16_t*>(block_data + offset);
            return std::string_view(block_data + offset + 2, len);
        }

        void DestroyString(IdType id) override {
            auto [block_idx, offset] = Decode(id);
            if (block_idx == m_cur_block_) {
                uint16_t len = *reinterpret_cast<const uint16_t*>(m_blocks_[block_idx] + offset);
                if (offset + len + 2 == BLOCK_BYTES - m_cur_remainder_) {
                    // 简单直接回收当前块末尾的字符串，无需加入空闲列表。不在考虑合并相邻空闲槽位的情况，
                    m_cur_remainder_ += (len + 2);
                    return;
                }
            }
            const char* block_data = m_blocks_[block_idx];
            uint16_t    len        = *reinterpret_cast<const uint16_t*>(block_data + offset);
            m_free_by_len_[len].push_back(id);
            // 注意：这里没有合并相邻的空闲槽位，可能会导致内存碎片，但是这里已经时长串了，合并的复杂度较高，
            // 且复用性不高
        }

        void Clear() override {
            for (auto& block : m_blocks_) {
                free(block);
            }
            m_blocks_.clear();
            m_free_by_len_.clear();
            m_cur_block_     = 0;
            m_cur_remainder_ = 0;
        }

    private:
        static std::pair<size_t, size_t> Decode(IdType id) {
            size_t block_idx = (id >> (OFFSET_BITS + 1)) & ((1ULL << BLOCK_BITS) - 1);
            size_t offset    = (id >> 1) & ((1ULL << OFFSET_BITS) - 1);
            return {block_idx, offset};
        }

        static IdType Encode(size_t block_idx, size_t offset) {
            IdType id = 1;
            id |= (static_cast<IdType>(offset) << 1);
            id |= (static_cast<IdType>(block_idx) << (OFFSET_BITS + 1));
            return id;
        }

        std::vector<char*>                      m_blocks_;
        std::map<uint16_t, std::vector<IdType>> m_free_by_len_;
        size_t                                  m_cur_block_     = 0;
        IdType                                  m_cur_remainder_ = 0;
    };

public:
    StringMgr() {
        m_string_tables_.emplace_back(std::make_unique<DynamicStringTable>());
        CreateFixedTables<MAX_INLINE_LEN>();
    }

    IdType AddString(const std::string& str) {
        size_t len = str.length();
        if (len > MAX_FIX_LENGTH) {
            return m_string_tables_[0]->AddString(str);
        }
        if (len < MAX_INLINE_LEN) {
            union {
                IdType  id;
                uint8_t bytes[sizeof(MAX_INLINE_LEN)];
            } u = {0};

            u.bytes[MAX_INLINE_LEN - 1] = static_cast<uint8_t>(len << 1);
            std::memcpy(u.bytes, str.data(), len);
            return u.id;
        }
        return GetFixedTable(len)->AddString(str);
    }

    std::string_view GetString(IdType& id) const {
        bool is_dynamic = (id & 1);
        if (!is_dynamic) {
            uint8_t len = (id >> 1) & 0x7F;
            if (len < MAX_INLINE_LEN) {
                union {
                    IdType  id;
                    uint8_t bytes[sizeof(MAX_INLINE_LEN)];
                } u = {id};
                return std::string_view(reinterpret_cast<const char*>(u.bytes), len);
            }
            return GetFixedTable(len)->GetString(id);
        } else {
            return m_string_tables_[0]->GetString(id);
        }
    }

    void DestroyString(const IdType& id) {
        bool is_dynamic = (id & 1);
        if (!is_dynamic) {
            uint8_t len = (id >> 1) & 0x7F;
            if (len < MAX_INLINE_LEN) return;
            GetFixedTable(len)->DestroyString(id);
        } else {
            m_string_tables_[0]->DestroyString(id);
        }
    }

    void Clear() {
        for (auto& tbl : m_string_tables_) {
            if (tbl != nullptr) {
                tbl->Clear();
            }
        }
    }

    void PrintOptimizationInfo() const {}

private:
    template <size_t Len>
    void CreateFixedTables() {
        static_assert(Len >= 1 && Len <= MAX_FIX_LENGTH);
        while (m_string_tables_.size() < Len - 1) {
            m_string_tables_.emplace_back(nullptr);
        }
        m_string_tables_.emplace_back(std::make_unique<FixLengthStringTable<Len>>());
        if constexpr (Len < MAX_FIX_LENGTH) {
            CreateFixedTables<Len + 1>();
        }
    }

    StringTableBase<IdType>* GetFixedTable(size_t len) {
        // m_string_tables_[0] 是动态表，[1] 对应长度1，[2] 对应长度2，...
        return m_string_tables_[len].get();
    }

    const StringTableBase<IdType>* GetFixedTable(size_t len) const {
        return m_string_tables_[len].get();
    }
    std::vector<std::unique_ptr<StringTableBase<IdType>>> m_string_tables_;
};

}  // namespace db

#endif