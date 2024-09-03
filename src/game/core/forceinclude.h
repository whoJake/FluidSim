#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <type_traits>
#include <sstream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <algorithm>
#include <format>
#include <limits>
#include <chrono>
#include <queue>

#include "system/types.h"

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/quaternion.hpp"
#include "gtx/hash.hpp"

#include "data/fixed_vector.h"

#include "pch/assert.h"

#include "system/param.h"
#include "system/log.h"

#define DEFAULT_COPY(classname) \
    classname(const classname&) = default;\
    classname& operator=(const classname&) = default

#define DEFAULT_MOVE(classname) \
    classname(classname&&) = default;\
    classname& operator=(classname&&) = default

#define DELETE_COPY(classname) \
    classname(const classname&) = delete;\
    classname& operator=(const classname&) = delete

#define DELETE_MOVE(classname) \
    classname(classname&&) = delete;\
    classname& operator=(classname&&) = delete