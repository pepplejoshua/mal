#pragma once

#include <map>
#include "mal_types.hpp"

using namespace std;

// allows us store all types of MalTypes
using Env = map < string, MalType* >;