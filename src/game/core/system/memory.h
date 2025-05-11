#pragma once

namespace sys
{

class memory
{
public:
    memory() = delete;
    ~memory() = delete;

    static constexpr u64 default_alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__;

    static void setup_heap();
    static void initialise_system_zones();

    static void* operator_new(u64 size, u64 align);
    static void operator_delete(void* ptr, u64 size);

    /// <summary>
    /// This exists as a method to guarentee that we track the allocated size of an array.
    /// Using this method for operator new overrides will increase the actual allocated size
    /// by sizeof(u64). operator_new_arr should only be deleted using operator_delete_array
    /// </summary>
    static void* operator_new_arr(u64 size, u64 align);

    /// <summary>
    /// This exists as a method to guarentee that we track the allocated size of an array.
    /// This should only be used on a pointer allocated with operator_new_arr. Without this
    /// the compiler offers no guarentee that operator[](void*, u64) is called so I'm just going
    /// to track array sizes myself with this.
    /// </summary>
    static void operator_delete_arr(void* ptr);
};

} // sys