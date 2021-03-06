#!/bin/bash
lower=`echo $1 | tr '[:upper:]' '[:lower:]'`
guard=DART_`echo $1 | tr '[:lower:]' '[:upper:]'`_`echo $1 | tr '[:lower:]' '[:upper:]'`_H_
cat <<END
/*
 * Copyright 2013 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#ifndef $guard
#define $guard

// Deprecated header file for case-sensitive filesystems
#warning The dart/$lower/$1.h header file is deprecated \\
  as of dart 4.3 and will be removed in dart 5.0. \\
  Please include dart/$lower/$2.h instead.
#include "dart/$lower/$2.h"

#endif

END
