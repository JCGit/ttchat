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
namespace uchat
{
/**
 * @note
 * - All cast MUST have explicit
 * - operator bool() cast: not for numeric but for object valid
 */
struct NumStrView : ::uchat::boost::StringView {
    using StringView = ::uchat::boost::StringView;
    constexpr NumStrView(): StringView() {}
    constexpr NumStrView(char const* const p, size_t const l) noexcept:
        StringView(p, l) {}
    /// @note object valid @sa toBool
    explicit operator bool() const noexcept { return this->p != nullptr; }
    /// @throw std::logic_error when cast nil value
    bool toBool() const {
        this->check();
        return bool(::atoll(this->p));
    }
    /// @throw std::logic_error when cast nil value
    explicit operator uint32_t() const {
        this->check();
        return uint32_t(::atoll(this->p));
    }
    /// @throw std::logic_error when cast nil value
    explicit operator int32_t() const {
        this->check();
        return int32_t(::atoll(this->p));
    }
};
}
