#pragma once

namespace sys
{

class allocator
{
public:
    ~allocator() = default;

    void* allocate(u64 size, u64 align);
    void free(void* ptr);

    virtual void* do_allocate(u64 size, u64 align) = 0;
    virtual void do_free(void* ptr) = 0;
protected:
    allocator() = default;
public:
    // Static interface
    static allocator* const get_main();
    static void set_main(allocator* ptr);

    static constexpr u64 default_align = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
private:
    inline static allocator* sm_main{ nullptr };

};

} // sys