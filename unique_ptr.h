#pragma once

#include <cstddef>
#include <memory>
#include <utility>

template <typename T, typename Deleter = std::default_delete<T>>
class UniquePtr {
public:
    // ======================== Constructors ========================

    UniquePtr();
    explicit UniquePtr(T* ptr);
    UniquePtr(T* ptr, const Deleter& deleter);
    UniquePtr(T* ptr, Deleter&& deleter);

    // =================== No copy ==================================

    UniquePtr(const UniquePtr&)            = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // =================== Move ====================================

    UniquePtr(UniquePtr&& other);
    UniquePtr& operator=(UniquePtr&& other);

    // =================== Destructor ==============================

    ~UniquePtr();

    // =================== Observers ===============================

    T* get();
    const T* get() const;

    explicit operator bool() const;

    T&       operator*();
    const T& operator*() const;

    T* operator->();
    const T* operator->() const;

    Deleter&       get_deleter();
    const Deleter& get_deleter() const;

    // =================== Modifiers ===============================

    T* release();
    void reset(T* ptr = nullptr);
    void swap(UniquePtr& other);

private:
    // Empty Base Optimization (EBO) helper
    struct CompressedPtr : Deleter {
        T* ptr;
        CompressedPtr() : Deleter(), ptr(nullptr) {}
        CompressedPtr(T* p) : Deleter(), ptr(p) {}
        CompressedPtr(T* p, const Deleter& d) : Deleter(d), ptr(p) {}
        CompressedPtr(T* p, Deleter&& d) : Deleter(std::move(d)), ptr(p) {}
    };

    CompressedPtr impl_;
};

// =====================================================================
//  Partial specialization for arrays: UniquePtr<T[]>
// =====================================================================

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    // ======================== Constructors ========================

    UniquePtr();
    explicit UniquePtr(T* ptr);
    UniquePtr(T* ptr, const Deleter& deleter);
    UniquePtr(T* ptr, Deleter&& deleter);

    // =================== No copy ==================================

    UniquePtr(const UniquePtr&)            = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // =================== Move ====================================

    UniquePtr(UniquePtr&& other);
    UniquePtr& operator=(UniquePtr&& other);

    // =================== Destructor ==============================

    ~UniquePtr();

    // =================== Observers ===============================

    T* get();
    const T* get() const;

    explicit operator bool() const;

    T&       operator[](size_t index);
    const T& operator[](size_t index) const;

    Deleter&       get_deleter();
    const Deleter& get_deleter() const;

    // =================== Modifiers ===============================

    T* release();
    void reset(T* ptr = nullptr);
    void swap(UniquePtr& other);

private:
    struct CompressedPtr : Deleter {
        T* ptr;
        CompressedPtr() : Deleter(), ptr(nullptr) {}
        CompressedPtr(T* p) : Deleter(), ptr(p) {}
        CompressedPtr(T* p, const Deleter& d) : Deleter(d), ptr(p) {}
        CompressedPtr(T* p, Deleter&& d) : Deleter(std::move(d)), ptr(p) {}
    };

    CompressedPtr impl_;
};

// =================== Free function ===============================

template <typename T, typename... Args>
UniquePtr<T> make_unique(Args&&... args);

// =====================================================================
//  IMPLEMENTATION
// =====================================================================

// Single object UniquePtr

template <typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr() : impl_() {}

template <typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(T* ptr) : impl_(ptr) {}

template <typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(T* ptr, const Deleter& deleter) : impl_(ptr, deleter) {}

template <typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(T* ptr, Deleter&& deleter) : impl_(ptr, std::move(deleter)) {}

template <typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(UniquePtr&& other) : impl_(other.release(), std::move(other.get_deleter())) {}

template <typename T, typename Deleter>
UniquePtr<T, Deleter>& UniquePtr<T, Deleter>::operator=(UniquePtr&& other) {
    if (this != &other) {
        reset(other.release());
        get_deleter() = std::move(other.get_deleter());
    }
    return *this;
}

template <typename T, typename Deleter>
UniquePtr<T, Deleter>::~UniquePtr() { reset(); }

template <typename T, typename Deleter>
T* UniquePtr<T, Deleter>::get() { return impl_.ptr; }

template <typename T, typename Deleter>
const T* UniquePtr<T, Deleter>::get() const { return impl_.ptr; }

template <typename T, typename Deleter>
UniquePtr<T, Deleter>::operator bool() const { return impl_.ptr != nullptr; }

template <typename T, typename Deleter>
T& UniquePtr<T, Deleter>::operator*() { return *impl_.ptr; }

template <typename T, typename Deleter>
const T& UniquePtr<T, Deleter>::operator*() const { return *impl_.ptr; }

template <typename T, typename Deleter>
T* UniquePtr<T, Deleter>::operator->() { return impl_.ptr; }

template <typename T, typename Deleter>
const T* UniquePtr<T, Deleter>::operator->() const { return impl_.ptr; }

template <typename T, typename Deleter>
Deleter& UniquePtr<T, Deleter>::get_deleter() { return static_cast<Deleter&>(impl_); }

template <typename T, typename Deleter>
const Deleter& UniquePtr<T, Deleter>::get_deleter() const { return static_cast<const Deleter&>(impl_); }

template <typename T, typename Deleter>
T* UniquePtr<T, Deleter>::release() {
    T* temp = impl_.ptr;
    impl_.ptr = nullptr;
    return temp;
}

template <typename T, typename Deleter>
void UniquePtr<T, Deleter>::reset(T* ptr) {
    T* old_ptr = impl_.ptr;
    impl_.ptr = ptr;
    if (old_ptr) get_deleter()(old_ptr);
}

template <typename T, typename Deleter>
void UniquePtr<T, Deleter>::swap(UniquePtr& other) {
    std::swap(impl_.ptr, other.impl_.ptr);
    std::swap(get_deleter(), other.get_deleter());
}

// Array specialization UniquePtr<T[]>

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::UniquePtr() : impl_() {}

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::UniquePtr(T* ptr) : impl_(ptr) {}

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::UniquePtr(T* ptr, const Deleter& deleter) : impl_(ptr, deleter) {}

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::UniquePtr(T* ptr, Deleter&& deleter) : impl_(ptr, std::move(deleter)) {}

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::UniquePtr(UniquePtr&& other) : impl_(other.release(), std::move(other.get_deleter())) {}

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>& UniquePtr<T[], Deleter>::operator=(UniquePtr&& other) {
    if (this != &other) {
        reset(other.release());
        get_deleter() = std::move(other.get_deleter());
    }
    return *this;
}

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::~UniquePtr() { reset(); }

template <typename T, typename Deleter>
T* UniquePtr<T[], Deleter>::get() { return impl_.ptr; }

template <typename T, typename Deleter>
const T* UniquePtr<T[], Deleter>::get() const { return impl_.ptr; }

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::operator bool() const { return impl_.ptr != nullptr; }

template <typename T, typename Deleter>
T& UniquePtr<T[], Deleter>::operator[](size_t index) { return impl_.ptr[index]; }

template <typename T, typename Deleter>
const T& UniquePtr<T[], Deleter>::operator[](size_t index) const { return impl_.ptr[index]; }

template <typename T, typename Deleter>
Deleter& UniquePtr<T[], Deleter>::get_deleter() { return static_cast<Deleter&>(impl_); }

template <typename T, typename Deleter>
const Deleter& UniquePtr<T[], Deleter>::get_deleter() const { return static_cast<const Deleter&>(impl_); }

template <typename T, typename Deleter>
T* UniquePtr<T[], Deleter>::release() {
    T* temp = impl_.ptr;
    impl_.ptr = nullptr;
    return temp;
}

template <typename T, typename Deleter>
void UniquePtr<T[], Deleter>::reset(T* ptr) {
    T* old_ptr = impl_.ptr;
    impl_.ptr = ptr;
    if (old_ptr) get_deleter()(old_ptr);
}

template <typename T, typename Deleter>
void UniquePtr<T[], Deleter>::swap(UniquePtr& other) {
    std::swap(impl_.ptr, other.impl_.ptr);
    std::swap(get_deleter(), other.get_deleter());
}

// make_unique

template <typename T, typename... Args>
UniquePtr<T> make_unique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}