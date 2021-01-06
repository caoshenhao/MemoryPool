#pragma once
#include<cstddef>
#include<cstdint>
#include<utility>
#include<new>
template <typename T, size_t BlockSize = 4096>
class MemoryPool {
public:
    template <typename U> struct rebind {typedef MemoryPool<U> other;};
    // 构造与析构
    MemoryPool() noexcept;
    MemoryPool(const MemoryPool &mp) noexcept;
    MemoryPool(MemoryPool &&mp) noexcept;

    // 其他类型作为参数的构造函数
    template<class U> MemoryPool(const MemoryPool<U> &mp) noexcept;

    ~MemoryPool() noexcept;

    // 禁用赋值
    MemoryPool &operator=(const MemoryPool &) = delete;
    MemoryPool &operator=(MemoryPool &&);

    // 获取元素地址
    inline T *addaress(T &element) const noexcept {return &element;}
    inline const T *addaress(const T &element) const noexcept {return &element;}

    // 实现接口
    inline T *allocate(size_t n=1, const T *hint=nullptr) {
        if(freeSlot_ != nullptr) {
            // freeSlot_ 为Slot_*
            T *result = reinterpret_cast<T *>(freeSlot_);
            freeSlot_ = freeSlot_->next;
            return result;
        }
        else {
            if(currentSlot_ >= lastSlot_) {
                allocateBlock();
            }
            return reinterpret_cast<T *>(currentSlot_++);
        }
    }

    inline void deallocate(T *p, size_t=1) {
        if(p != nullptr) {
            reinterpret_cast<Slot_ *>(p)->next = freeSlot_;
            freeSlot_ = reinterpret_cast<Slot_ *>(p);
        }
    }

    template <typename U, typename ...Args>
    inline void construct(U *p, Args &&... args) {
        new(p) U(std::forward<Args>(args)...);
    }
    template <typename U> inline void destroy(U *p) {p->~U();}

    // new 和 delete
    template<typename... Args> inline T *newElement(Args &&... args) {
        T *result = allocate();
        construct(result, std::forward<Args>(args)...);
        return result;
    }
    inline void deleteElement(T *p) {
        if(p != nullptr) {
            p->~T();
            deallocate(p);
        }
    }

    // 最多能容纳多少个对象
    inline size_t max_size() const noexcept {
        size_t maxBlocks = -1 / BlockSize;
        return (BlockSize-sizeof(char *)) / sizeof(Slot_) * maxBlocks;
    }

private:

    union Slot_ {
        T element;
        Slot_ *next;
    };

    Slot_ *currentBlock_;
    Slot_ *currentSlot_;
    Slot_ *lastSlot_;
    Slot_ *freeSlot_; // 指向deallocate后空闲的slots

    inline size_t padPointer(char *p, size_t algin) const noexcept {
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        size_t padding = (algin - addr) % algin;
        return padding;
    }

    void allocateBlock();

    // 编译时断言检查
    static_assert(BlockSize >= 2 * sizeof(Slot_), "BlockSize too small.");
};

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool() noexcept {
    currentBlock_ = nullptr;
    currentSlot_ = nullptr;
    lastSlot_ = nullptr;
    freeSlot_ = nullptr;
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool &mp) noexcept
    : MemoryPool() {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(MemoryPool &&mp) noexcept {
    currentBlock_ = mp.currentBlock_;
    mp.currentBlock_ = nullptr;
    currentSlot_ = mp.currentSlot_;
    lastSlot_ = mp.lastSlot_;
    freeSlot_ = mp.freeSlot_;
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() noexcept {
    Slot_ *cur = currentSlot_;
    while(cur != nullptr) {
        Slot_ *nextBlock = cur->next;
        operator delete(reinterpret_cast<void *>(cur));
        cur = nextBlock;
    }
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::allocateBlock() {
    char *newBlock = reinterpret_cast<char *>(operator new(BlockSize));
    reinterpret_cast<Slot_ *>(newBlock)->next = currentBlock_;
    currentBlock_ = reinterpret_cast<Slot_ *>(newBlock);

    char *body = newBlock + sizeof(Slot_*);
    size_t bodyPadding = padPointer(body, alignof(Slot_));
    currentSlot_ = reinterpret_cast<Slot_ *>(body + bodyPadding);

    lastSlot_ = reinterpret_cast<Slot_ *>(newBlock + BlockSize - sizeof(Slot_) + 1);
}