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
#include "uchat/errno.hpp"
#include <string.h>
namespace uchat
{
std::string ErrnoToString(Errno const& e) noexcept
{
    switch(e) {
#   define __CASE(c) case Errno::c: return #c "(" \
        + std::to_string(int32_t(Errno::c)) + ")";
    __CASE(Ok)
    __CASE(NotSet)
    __CASE(InvalidType)
    __CASE(BadAnyCast)
    __CASE(AlreadyOpen)
    __CASE(InvalidFilename)
    __CASE(InvalidData)
    __CASE(InvalidParam)
    __CASE(OpenError)
    __CASE(StatError)
    __CASE(NotOpen)
    __CASE(NoMem)
    __CASE(ReadError)
    __CASE(Cancelled)
    __CASE(Busy)
    __CASE(Interrupt)
    __CASE(SeekError)
    __CASE(FileChanged)
#   undef __CASE
    default:
        if (static_cast<int32_t>(e) >= kCppbaseErrnoOffset) {
            int32_t const stde = int32_t(e) - kCppbaseErrnoOffset;
            return ::strerror(stde) + std::string("(") + std::to_string(stde)
                + ")";
        } else {
            return "(unknown errno)(" + std::to_string(int32_t(e)) + ")";
        }
    }
}
}
