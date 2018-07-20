/* This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#pragma once
#include "uchat/stringview.hpp"
#include "uchat/numericstringview.hpp"
#include <vector>
namespace uchat
{
/**
 * @sa
 * - boost::variant
 * - C++17 https://zh.cppreference.com/w/cpp/utility/variant
 */
struct SimpleVariantView {
    using StringView = ::uchat::boost::StringView;
    constexpr SimpleVariantView() noexcept:
        data(nullptr), size(0u) {}
    SimpleVariantView(void const* const data, uint64_t const size) noexcept:
        data(data), size(size) {}
    inline operator bool() const noexcept { return this->data != nullptr; }
    template<typename T = StringView>
    typename std::enable_if<
        std::is_same<StringView, typename std::decay<T>::type>::value,
        StringView>::type as() const noexcept;
    template<typename T = NumStrView>
    typename std::enable_if<
        std::is_same<NumStrView, typename std::decay<T>::type>::value,
        NumStrView>::type as() const noexcept;
    inline void reset() noexcept {
        this->data = nullptr;
        this->size = 0u;
    }
protected:
    void const* data{ nullptr };
    uint64_t size{ 0u };
};
template<typename T>
typename std::enable_if<
    std::is_same<SimpleVariantView::StringView,
    typename std::decay<T>::type>::value,
    SimpleVariantView::StringView>::type SimpleVariantView::as() const noexcept
{
    if (*this) {
        return StringView {
            reinterpret_cast<char const*>(this->data), this->size };
    } else {
        return StringView{};
    }
}
template<typename T>
typename std::enable_if<
    std::is_same<NumStrView, typename std::decay<T>::type>::value,
    NumStrView>::type SimpleVariantView::as() const noexcept
{
    if (*this) {
        return NumStrView {
            reinterpret_cast<char const*>(this->data), this->size };
    } else {
        return NumStrView{};
    }
}
}
