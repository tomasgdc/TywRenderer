#pragma once

#define _CRTDBG_MAP_ALLOC


#include <Windows.h>
#include <Windowsx.h>
#include <mmsystem.h>

#include <iomanip>
#include <limits>
#include <queue>
#include <cassert>
#include <cmath>
#include <string>
#include <stdlib.h>
#include <crtdbg.h>
#include <cstdint>
#include <cstdio>
#include <chrono>
#include <windows.h>
#include <fileapi.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <set>
#include <array>
#include <map>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <sstream>
#include <ostream>
#include <strstream>
#include "macro.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <External/gli/gli/gli.hpp>
#include <External/glm/glm/glm.hpp>
#include <External/glm/glm/gtc/matrix_transform.hpp>
#include <External/glm/glm/gtc/type_ptr.hpp>
#include <External/glm/glm/gtc/matrix_inverse.hpp>
#include <External/glm/glm/gtx/quaternion.hpp> 
#include <External/glm/glm/gtx/compatibility.hpp> 
#include <External/glm/glm/gtx/euler_angles.hpp>
#include <External/glm/glm/gtx/norm.hpp>
#include <External/glm/glm/gtx/transform.hpp>
#include <External/glm/glm/gtx/matrix_operation.hpp>
#include <External\glm\glm\gtx\orthonormalize.hpp>