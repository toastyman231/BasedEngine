#pragma once

// STL stuff
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <type_traits>
#include <set>
#include <array>
#include <queue>
#include <stack>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <random>
#include <filesystem>
#include <cstdio>
#include <format>
#include <concepts>

// GLM stuff
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

// RML stuff
#include <RmlUi/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Debugger.h>

// entt stuff
#include <entt/entt.hpp>

// jolt stuff
#include <Jolt/Jolt.h>

// Based stuff
// TODO: Add this stuff back in
/*#include "based/log.h"
#include "based/core/profiler.h"
#include "based/graphics/glmhelpers.h"
#include "based/graphics/helpers.h"
#include "based/memory/memoryhelpers.h"*/

// Platform stuff
#ifdef BASED_PLATFORM_WINDOWS
#include <Windows.h>
#endif