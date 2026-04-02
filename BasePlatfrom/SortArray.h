#ifndef BASEPLATFROM_SORTARRAY_H_
#define BASEPLATFROM_SORTARRAY_H_

#include <algorithm>
#include <cstring>
#include <stdexcept>

template <typename T, typename Compare = std::less<T>>
class SortArray {
private:
    T*      data_;
    size_t  capacity_;
    size_t  size_;
    Compare comp_;

    void Resize(size_t new_capacity) {
        T* new_data = new T[new_capacity];
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }
        delete[] data_;

        data_     = new_data;
        capacity_ = new_capacity;
    }

public:
    SortArray() : data_(nullptr), capacity_(0), size_(0) {}
    SortArray(T* data, size_t size)
        : data_(data), capacity_(size), size_(size) {}
    explicit SortArray(const Compare& comp)
        : data_(nullptr), capacity_(0), size_(0), comp_(comp) {}

    ~SortArray() {
        Clear();
        delete[] data_;
    }
    void SetCompare(const Compare& comp) { comp_ = comp; }
    SortArray(const SortArray&)            = delete;
    SortArray& operator=(const SortArray&) = delete;

    SortArray(SortArray&& other) noexcept
        : data_(other.data_),
          capacity_(other.capacity_),
          size_(other.size_),
          comp_(std::move(other.comp_)) {
        other.data_     = nullptr;
        other.capacity_ = 0;
        other.size_     = 0;
    }

    SortArray& operator=(SortArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_           = other.data_;
            capacity_       = other.capacity_;
            size_           = other.size_;
            comp_           = std::move(other.comp_);
            other.data_     = nullptr;
            other.capacity_ = 0;
            other.size_     = 0;
        }
        return *this;
    }

    void Insert(const T& value) {
        size_t pos = LowerBoundIndex(value);
        if (size_ >= capacity_) {
            size_t new_capacity = capacity_ == 0 ? 8 : capacity_ * 2;
            Resize(new_capacity);
        }
        for (size_t i = size_; i > pos; --i) {
            data_[i] = std::move(data_[i - 1]);
        }
        data_[pos] = value;
        ++size_;
    }

    void Insert(T&& value) {
        size_t pos = LowerBoundIndex(value);

        if (size_ >= capacity_) {
            size_t new_capacity = capacity_ == 0 ? 8 : capacity_ * 2;
            Resize(new_capacity);
        }

        for (size_t i = size_; i > pos; --i) {
            data_[i] = std::move(data_[i - 1]);
        }

        data_[pos] = std::move(value);
        ++size_;
    }

    void PushBack(const T& value) {
        if (size_ >= capacity_) {
            size_t new_capacity = capacity_ == 0 ? 8 : capacity_ * 2;
            Resize(new_capacity);
        }
        data_[size_++] = value;
    }

    bool Erase(const T& value) {
        size_t pos = FindIndex(value);
        if (pos == size_) return false;

        for (size_t i = pos; i < size_ - 1; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }

        --size_;
        return true;
    }

    void EraseAt(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }

        for (size_t i = index; i < size_ - 1; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }

        --size_;
    }

    size_t FindIndex(const T& value) const {
        size_t left  = 0;
        size_t right = size_;

        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (comp_(data_[mid], value)) {
                left = mid + 1;
            } else if (comp_(value, data_[mid])) {
                right = mid;
            } else {
                return mid;
            }
        }
        return size_;
    }

    /**
     * Finds the index of the first element that is not less than the given
     * value. if all elements are less than the value, returns size_.
     */
    size_t FindLowerBoundIndex(const T& value) const {
        size_t left  = 0;
        size_t right = size_;
        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (comp_(data_[mid], value)) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }
        return left;
    }

    /**
     * Finds the index of the first element that is not less than the given
     * value. if all elements are less than the value, returns size_.
     */
    size_t FindLowerBoundIndex(const T& value) {
        size_t left  = 0;
        size_t right = size_;
        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (comp_(data_[mid], value)) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }
        return left;
    }

    /**
     * Finds the index of the first element that is greater than the given
     * value. if no elements are greater than the value, returns size_.
     */
    size_t FindUpperBoundIndex(const T& value) const {
        size_t left  = 0;
        size_t right = size_;
        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (comp_(value, data_[mid])) {
                right = mid;
            } else {
                left = mid + 1;
            }
        }
        return left;
    }

    /**
     * Finds the index of the first element that is greater than the given
     * value. if no elements are greater than the value, returns size_.
     */
    size_t FindUpperBoundIndex(const T& value) {
        size_t left  = 0;
        size_t right = size_;
        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (comp_(value, data_[mid])) {
                right = mid;
            } else {
                left = mid + 1;
            }
        }
        return left;
    }

    T* Find(const T& value) {
        size_t index = FindIndex(value);
        return index < size_ ? &data_[index] : nullptr;
    }

    const T* Find(const T& value) const {
        size_t index = FindIndex(value);
        return index < size_ ? &data_[index] : nullptr;
    }

    bool Contains(const T& value) const { return FindIndex(value) < size_; }

    size_t LowerBoundIndex(const T& value) const {
        size_t left  = 0;
        size_t right = size_;

        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (comp_(data_[mid], value)) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }

        return left;
    }

    size_t UpperBoundIndex(const T& value) const {
        size_t left  = 0;
        size_t right = size_;

        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (comp_(value, data_[mid])) {
                right = mid;
            } else {
                left = mid + 1;
            }
        }

        return left;
    }

    T& operator[](size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }

    const T& operator[](size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }

    size_t Size() const { return size_; }
    size_t Capacity() const { return capacity_; }
    bool   Empty() const { return size_ == 0; }

    void Reserve(size_t capacity) {
        if (capacity > capacity_) {
            Resize(capacity);
        }
    }

    void Clear() {
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = 0;
    }

    T*       Data() { return data_; }
    const T* Data() const { return data_; }

    T*       Begin() { return data_; }
    T*       End() { return data_ + size_; }
    const T* Begin() const { return data_; }
    const T* End() const { return data_ + size_; }

    T*       begin() { return data_; }
    T*       end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }
};

#endif  // BASEPLATFROM_SORTARRAY_H_