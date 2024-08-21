#pragma once

#include <mutex>

std::string get_thread_name(std::thread::id id = std::this_thread::get_id());

uint32_t get_thread_id(std::thread::id id = std::this_thread::get_id());

std::thread request_thread(std::string name, std::function<void()> function);