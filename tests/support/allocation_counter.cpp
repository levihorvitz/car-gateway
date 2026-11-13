#include "support/allocation_counter.hpp"

#include <atomic>
#include <cstdlib>
#include <new>

namespace {

std::atomic<std::size_t> gAllocations{0};

}  // namespace

void* operator new(std::size_t size) {
    gAllocations.fetch_add(1, std::memory_order_relaxed);
    if (size == 0) {
        size = 1;
    }
    if (void* memory = std::malloc(size)) {
        return memory;
    }
    throw std::bad_alloc();
}

void operator delete(void* memory) noexcept {
    std::free(memory);
}

void operator delete(void* memory, std::size_t) noexcept {
    std::free(memory);
}

namespace gateway::test {

std::size_t allocationCount() noexcept {
    return gAllocations.load(std::memory_order_relaxed);
}

}  // namespace gateway::test
