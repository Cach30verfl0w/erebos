//   Copyright 2024 Cach30verfl0w
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/**
 * @author Cedric Hammes
 * @since  18/03/2024
 */

#include <erebos/unicode.hpp>
#include <gtest/gtest.h>

TEST(erebos_unicode, to_wcs) {
    ASSERT_EQ(erebos::unicode::to_wcs(R"(This is a test 🐺)"), LR"(This is a test 🐺)");
}

TEST(erebos_unicode, to_mbs) {
    ASSERT_EQ(erebos::unicode::to_mbs(LR"(This is a test 🐺)"), R"(This is a test 🐺)");
}

