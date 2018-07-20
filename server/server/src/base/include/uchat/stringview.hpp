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
#include <string.h>
#include <string>
#include <ostream>
namespace uchat
{
namespace boost
{
/**
 * boost::string_ref specfic string view
 * @sa
 * - boost::string_ref
 * - C++17 http://zh.cppreference.com/w/cpp/string/basic_string_view
 */
struct StringView {
    constexpr StringView() noexcept = default;
    constexpr StringView(char const* const p, size_t const l) noexcept:
        p(p), l(l) {}
    constexpr StringView(char const* const p) noexcept:
        p(p), l(::strlen(p)) {}
    StringView(std::string const& s) noexcept: p(s.data()), l(s.length()) {}
    virtual ~StringView() noexcept = default;
    /// @throw std::logic_error when this->p is nil
    void check() const {
        if (!this->p) {
            throw std::logic_error(std::string(__FILE__)
                + "+" + std::to_string(__LINE__)
                + ": got nil value");
        }
    }
    size_t size() const noexcept { return this->l; }
    size_t length() const noexcept { return this->l; }
    bool empty() const noexcept { return 0 == this->l; }
    char const* data() const noexcept { return this->p; }
    inline operator bool() const { return this->p != nullptr; }
    /// @throw std::logic_error when this->p is nil
    explicit operator std::string() const {
        this->check();
        return std::string(this->p, this->l);
    }
    /// @throw std::logic_error when this->p is nil
    std::string to_string() const {
        this->check();
        return std::string(this->p, this->l);
    }
    int compare(StringView const& x) const noexcept {
        const int cmp = ::strncmp(this->p, x.p, std::min(this->l, x.l));
        return cmp != 0 ? cmp : (this->l == x.l ? 0 : this->l < x.l ? -1 : 1);
    }
    inline bool operator==(StringView const& y) const noexcept {
        if (this->l != y.l) {
            return false;
        }
        return this->compare(y) == 0;
    }
    inline friend bool operator==(char const* const x, StringView const y) {
        return StringView(x) == y;
    }
    friend std::ostream& operator<<(std::ostream& os, StringView const& s)
    noexcept {
        if (os.good()) {
            std::size_t const size = s.size();
            std::size_t const w = static_cast< std::size_t >(os.width());
            if (w <= size) {
                os.write(s.data(), size);
            } else {
                //detail::insert_aligned(os, s);
                os << s.data();
            }
            os.width(0);
        }
        return os;
    }
protected:
    char const* p{ nullptr };
    size_t l{ 0 };
};
}
}
