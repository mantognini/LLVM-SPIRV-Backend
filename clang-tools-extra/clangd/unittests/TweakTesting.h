//===--- TweakTesting.h - Test helpers for refactoring actions ---*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_UNITTESTS_CLANGD_TWEAKTESTING_H
#define LLVM_CLANG_TOOLS_EXTRA_UNITTESTS_CLANGD_TWEAKTESTING_H

#include "TestTU.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace clang {
namespace clangd {

// Fixture base for testing tweaks. Intended to be subclassed for each tweak.
//
// Usage:
// TWEAK_TEST(ExpandAutoType);
//
// TEST_F(ExpandAutoTypeTest, ShortensTypes) {
//   Header = R"cpp(
//     namespace foo { template<typename> class X{}; }
//     using namespace foo;
//   cpp)";
//   Context = Block;
//   EXPECT_THAT(apply("[[auto]] X = foo<int>();"),
//               "foo<int> X = foo<int();");
//   EXPECT_AVAILABLE("^a^u^t^o^ X = foo<int>();");
//   EXPECT_UNAVAILABLE("auto ^X^ = ^foo<int>();");
// }
class TweakTest : public ::testing::Test {
  const char *TweakID;

public:
  // Inputs are wrapped in file boilerplate before attempting to apply a tweak.
  // Context describes the type of boilerplate.
  enum CodeContext {
    // Code snippet is placed directly into the source file. e.g. a declaration.
    File,
    // Snippet will appear within a function body. e.g. a statement.
    Function,
    // Snippet is an expression.
    Expression,
  };

protected:
  TweakTest(const char *TweakID) : TweakID(TweakID) {}

  // Contents of a header file to be implicitly included.
  // This typically contains declarations that will be used for a set of related
  // testcases.
  std::string Header;

  // Context in which snippets of code should be placed to run tweaks.
  CodeContext Context = File;

  // Apply the current tweak to the range (or point) in MarkedCode.
  // MarkedCode will be wrapped according to the Context.
  //  - if the tweak produces edits, returns the edited code (without markings).
  //    The context added to MarkedCode will be stripped away before returning,
  //    unless the tweak edited it.
  //  - if the tweak produces a message, returns "message:\n<message>"
  //  - if prepare() returns false, returns "unavailable"
  //  - if apply() returns an error, returns "fail: <message>"
  std::string apply(llvm::StringRef MarkedCode) const;

  // Accepts a code snippet with many ranges (or points) marked, and returns a
  // list of snippets with one range marked each.
  // Primarily used from EXPECT_AVAILABLE/EXPECT_UNAVAILABLE macro.
  static std::vector<std::string> expandCases(llvm::StringRef MarkedCode);

  // Returns a matcher that accepts marked code snippets where the tweak is
  // available at the marked range.
  ::testing::Matcher<llvm::StringRef> isAvailable() const;
};

#define TWEAK_TEST(TweakID)                                                    \
  class TweakID##Test : public ::clang::clangd::TweakTest {                    \
  protected:                                                                   \
    TweakID##Test() : TweakTest(#TweakID) {}                                   \
  }

#define EXPECT_AVAILABLE(MarkedCode)                                           \
  do {                                                                         \
    for (const auto &Case : expandCases(MarkedCode))                           \
      EXPECT_THAT(Case, ::clang::clangd::TweakTest::isAvailable());            \
  } while (0)

#define EXPECT_UNAVAILABLE(MarkedCode)                                         \
  do {                                                                         \
    for (const auto &Case : expandCases(MarkedCode))                           \
      EXPECT_THAT(Case,                                                        \
                  ::testing::Not(::clang::clangd::TweakTest::isAvailable()));  \
  } while (0)

} // namespace clangd
} // namespace clang

#endif
