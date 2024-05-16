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

#include "logging/jclog.h"
#include "pch/logging.h"
#include "pch/assert.h"
#include "Params.h"