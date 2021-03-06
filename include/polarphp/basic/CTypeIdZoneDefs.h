//===--- CTypeIDZone.def - Define the C++ TypeID Zone -----------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
//  This definition file describes the types  in the "C++" TypeID zone,
//  for use with the TypeID template.
//
//===----------------------------------------------------------------------===//

#ifndef
# define POLAR_TYPEID_NAMED(ctype, polarType)
#endif

// C types.
POLAR_TYPEID_NAMED(unsigned char, UnsignedChar)
POLAR_TYPEID_NAMED(signed char, SignedChar)
POLAR_TYPEID_NAMED(char, Char)
POLAR_TYPEID_NAMED(short, Short)
POLAR_TYPEID_NAMED(unsigned short, UnsignedShort)
POLAR_TYPEID_NAMED(int, Int)
POLAR_TYPEID_NAMED(unsigned int, UnsignedInt)
POLAR_TYPEID_NAMED(long, Long)
POLAR_TYPEID_NAMED(unsigned long, UnsignedLong)
POLAR_TYPEID_NAMED(long long, LongLong)
POLAR_TYPEID_NAMED(unsigned long long, UnsignedLongLong)
POLAR_TYPEID_NAMED(float, Float)
POLAR_TYPEID_NAMED(double, Double)
POLAR_TYPEID_NAMED(bool, Bool)
POLAR_TYPEID_NAMED(decltype(nullptr), NullPtr)
POLAR_TYPEID_NAMED(void, Void)
POLAR_TYPEID_NAMED(std::string, String)

#ifndef POLAR_TYPEID_TEMPLATE1_NAMED
# define POLAR_TYPEID_TEMPLATE1_NAMED(T1, T2, T3, T4)
#endif

// C++ standard library types.
POLAR_TYPEID_TEMPLATE1_NAMED(std::vector, Vector, typename T, T)
// polar ADT types.
POLAR_TYPEID_TEMPLATE1_NAMED(polar::basic::TinyPtrVector, TinyPtrVector, typename T, T)

