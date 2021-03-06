//===--- SimpleRequest.h - Simple Request Instances -------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//===----------------------------------------------------------------------===//
// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarphp software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by polarboy on 2019/05/05.
//===----------------------------------------------------------------------===//
//
//  This file defines the SimpleRequest class template, which makes it easier
//  to define new request kinds.
//
//===----------------------------------------------------------------------===//

#ifndef POLAR_AST_SIMPLEREQUEST_H
#define POLAR_AST_SIMPLEREQUEST_H

#include "polarphp/ast/DiagnosticEngine.h"
#include "polarphp/ast/DiagnosticsCommon.h"
#include "polarphp/basic/SimpleDisplay.h"
#include "polarphp/basic/LangStatistic.h"
#include "polarphp/Basic/TypeId.h"
#include "polarphp/basic/adt/Hashing.h"
#include "polarphp/basic/adt/StlExtras.h"
#include <tuple>

namespace polar::ast {

using polar::basic::UnifiedStatsReporter;
using polar::basic::FrontendStatsTracer;

class Evaluator;

/// Describes how the result for a particular request will be cached.
enum class CacheKind
{
   /// The result for a particular request should never be cached.
   Uncached,
   /// The result for a particular request should be cached within the
   /// evaluator itself.
   Cached,
   /// The result of a particular request will be cached via some separate
   /// mechanism, such as a mutable data structure.
   SeparatelyCached,
};

/// CRTP base class that describes a request operation that takes values
/// with the given input types (\c Inputs...) and produces an output of
/// the given type.
///
/// \tparam Derived The final, derived class type for the request.
/// \tparam Caching Describes how the output value is cached, if at all.
/// \tparam Output The type of the result produced by evaluating this request.
/// \tparam Inputs The types of the inputs to this request, i.e., the values
/// that comprise the request itself. These will determine the uniqueness of
/// the request.
///
/// The \c Derived class needs to implement several operations. The most
/// important one takes an evaluator and the input values, then computes the
/// final result, optionally bubbling up errors from recursive evaulations:
/// \code
///   llvm::Expected<Output> evaluate(Evaluator &evaluator, Inputs...) const;
/// \endcode
///
/// Cycle diagnostics can be handled in one of two ways. Either the \c Derived
/// class can implement the two cycle-diagnosing operations directly:
/// \code
///   void diagnoseCycle(DiagnosticEngine &diags) const;
///   void noteCycleStep(DiagnosticEngine &diags) const;
/// \endcode
///
/// Or the \c Derived class can provide a "diagnostic location" operation and
/// diagnostic values for the main cycle diagnostic and a "note" describing a
/// step within the chain of diagnostics:
/// \code
///   T getCycleDiagnosticLoc(Inputs...) const;
///   static constexpr Diag<Inputs...> cycleDiagnostic = ...;
///   static constexpr Diag<Inputs...> cycleStepDiagnostic = ...;
/// \endcode
///
/// Value caching is determined by the \c Caching parameter. When
/// \c Caching == CacheKind::SeparatelyCached, the \c Derived class is
/// responsible for implementing the two operations responsible to managing
/// the cache:
/// \code
///   Optional<Output> getCachedResult() const;
///   void cacheResult(Output value) const;
/// \endcode
template<typename Derived, CacheKind Caching, typename Output,
         typename ...Inputs>
class SimpleRequest
{
   std::tuple<Inputs...> m_storage;

   Derived &asDerived()
   {
      return *static_cast<Derived *>(this);
   }

   const Derived &asDerived() const
   {
      return *static_cast<const Derived *>(this);
   }

   template<size_t ...Indices>
   polar::utils::Expected<Output>
   callDerived(Evaluator &evaluator, polar::basic::index_sequence<Indices...>) const
   {
      static_assert(sizeof...(Indices) > 0, "Subclass must define evaluate()");
      return asDerived().evaluate(evaluator, std::get<Indices>(m_storage)...);
   }

   template<size_t ...Indices>
   void diagnoseImpl(DiagnosticEngine &diags, Diag<Inputs...> diag,
                     polar::basic::index_sequence<Indices...>) const
   {
      diags.diagnose(
               asDerived().getCycleDiagnosticLoc(std::get<Indices>(m_storage)...),
               diag, std::get<Indices>(m_storage)...);
   }

protected:
   /// Retrieve the storage value directly.
   const std::tuple<Inputs...> &getStorage() const
   {
      return m_storage;
   }

public:
   static const bool isEverCached = (Caching != CacheKind::Uncached);
   static const bool hasExternalCache = (Caching == CacheKind::SeparatelyCached);

   using OutputType = Output;

   explicit SimpleRequest(const Inputs& ...inputs)
      : m_storage(inputs...) { }

   /// Request evaluation function that will be registered with the evaluator.
   static polar::utils::Expected<OutputType>
   evaluateRequest(const Derived &request, Evaluator &evaluator)
   {
      return request.callDerived(evaluator,
                                 polar::basic::index_sequence_for<Inputs...>());
   }

   void diagnoseCycle(DiagnosticEngine &diags) const
   {
      diagnoseImpl(diags, Derived::cycleDiagnostic,
                   polar::basic::index_sequence_for<Inputs...>());
   }

   void noteCycleStep(DiagnosticEngine &diags) const
   {
      diagnoseImpl(diags, Derived::cycleStepDiagnostic,
                   polar::basic::index_sequence_for<Inputs...>());
   }

   friend bool operator==(const SimpleRequest &lhs, const SimpleRequest &rhs)
   {
      return lhs.m_storage == rhs.m_storage;
   }

   friend bool operator!=(const SimpleRequest &lhs, const SimpleRequest &rhs)
   {
      return !(lhs == rhs);
   }

   friend polar::basic::HashCode hash_value(const SimpleRequest &request)
   {
      using polar::basic::hash_combine;

      return hash_combine(polar::basic::TypeId<Derived>::value, request.m_storage);
   }

   friend void simple_display(RawOutStream &out,
                              const Derived &request)
   {
      out << polar::basic::TypeId<Derived>::getName();
      simple_display(out, request.m_storage);
   }

   friend class FrontendStatsTracer
   make_tracer(UnifiedStatsReporter *reporter, const Derived &request)
   {
      return make_tracer(reporter, polar::basic::TypeId<Derived>::getName(), request.m_storage);
   }
};

} // polar::ast

#endif // POLAR_AST_SIMPLEREQUEST_H
