//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// attribute.cc --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 16:02:53 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul  3 16:04:14 2011
// Update Count     : 52
//
// This  library is free  software; you  can redistribute  it and/or  modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software  Foundation; either  version 2.1 of  the License, or  (at your
// option) any later version.
//
// This library is distributed in the  hope that it will be useful, but WITHOUT
// ANY  WARRANTY;  without even  the  implied  warranty  of MERCHANTABILITY  or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should  have received a  copy of the  GNU Lesser General  Public License
// along  with this library.
//

#include "attribute.h"

#include <cstddef>					// NULL

attribute_t::attribute_t() {
    Mutex = false;
    dclkind.value = dclmutex.value = dclqual.value = 0;
    rttskkind.value = 0;
    typedef_base = NULL;
    focus = NULL;
    emptytemplate = emptyparms = nestedqual = false;
    plate = NULL;
    startT = endT = startMRP = startCR = startI = startE = startM = endM = startP = endP = NULL;
} // attribute_t::attribute_t

attribute_t::~attribute_t() {
    Mutex = false;
    dclkind.value = dclmutex.value = dclqual.value = 0;
    rttskkind.value = 0;
    typedef_base = NULL;
    focus = NULL;
    emptyparms = nestedqual = false;
    plate = NULL;
    startT = endT = startMRP = startCR = startI = startE = startM = endM = startP = endP = NULL;
} // attribute_t::~attribute_t

// Local Variables: //
// compile-command: "make install" //
// End: //
