#include "support/allocation_counter.hpp"

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>

namespace {

std::atomic<std::size_t> gAllocations{0};

void* allocate(std::size_t size) noexcept {
    gAllocations.fetch_add(1, std::memory_order_relaxed);
    return std::malloc(size == 0 ? 1 : size);
}

void* allocateAligned(std::size_t size, std::align_val_t alignment) noexcept {
    gAllocations.fetch_add(1, std::memory_order_relaxed);
    const auto align = static_cast<std::size_t>(alignment);
    const std::size_t rounded = ((size == 0 ? 1 : size) + align - 1) / align * align;
    return std::aligned_alloc(align, rounded);
}

void* orThrow(void* memory) {
    if (memory == nullptr) {
        throw std::bad_alloc();
    }
    return memory;
}

}  // namespace

void* operator new(std::size_t size) { return orThrow(allocate(size)); }
void* operator new[](std::size_t size) { return orThrow(allocate(size)); }
void* operator new(std::size_t size, const std::nothrow_t&) noexcept { return allocate(size); }
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept { return allocate(size); }

void* operator new(std::size_t size, std::align_val_t alignment) { return orThrow(allocateAligned(size, alignment)); }
void* operator new[](std::size_t size, std::align_val_t alignment) { return orThrow(allocateAligned(size, alignment)); }
void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return allocateAligned(size, alignment);
}
void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return allocateAligned(size, alignment);
}

void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t) noexcept { std::free(memory); }
void operator delete(void* memory, const std::nothrow_t&) noexcept { std::free(memory); }
void operator delete[](void* memory, const std::nothrow_t&) noexcept { std::free(memory); }

void operator delete(void* memory, std::align_val_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::align_val_t) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t, std::align_val_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t, std::align_val_t) noexcept { std::free(memory); }
void operator delete(void* memory, std::align_val_t, const std::nothrow_t&) noexcept { std::free(memory); }
void operator delete[](void* memory, std::align_val_t, const std::nothrow_t&) noexcept { std::free(memory); }

namespace gateway::test {

std::size_t allocationCount() noexcept {
    return gAllocations.load(std::memory_order_relaxed);
}

}  // namespace gateway::test
