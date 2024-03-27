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

#include <erebos/result.hpp>
#include <gtest/gtest.h>

TEST(erebos_Result, is_ok) {
    erebos::Result<erebos::u32> value = 1;
    static_assert(std::is_same_v<typename decltype(value)::value_type, erebos::u32>);
    static_assert(std::is_same_v<typename decltype(value)::error_type, std::string>);
    ASSERT_TRUE(value.is_ok());
}

TEST(erebos_Result, forward) {
    erebos::Result<erebos::u32> value = erebos::Error<std::string>("Test");
    static_assert(std::is_same_v<typename decltype(value)::value_type, erebos::u32>);
    static_assert(std::is_same_v<typename decltype(value)::error_type, std::string>);

    erebos::Result<erebos::u8> forwarded = value.forward<erebos::u8>();
    static_assert(std::is_same_v<typename decltype(forwarded)::value_type, erebos::u8>);
    static_assert(std::is_same_v<typename decltype(forwarded)::error_type, std::string>);

    ASSERT_TRUE(forwarded.is_error());
}