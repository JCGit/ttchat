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
#include <string>
namespace uchat
{
namespace errnum
{
constexpr int32_t kCannotConnectDatabase = -101;
constexpr int32_t kDatabaseQueryError    = -102;
constexpr int32_t kUserIdNotFound        = -103;
constexpr int32_t kInvalidParameter      = -201;
constexpr int32_t kException             = -999;
}
enum class Errno: int32_t {
    Ok = 0, ///< success
    NotSet, ///< optional or related not set
    InvalidType, ///< invalid type
    BadAnyCast, ///< any cast fail
    AlreadyOpen, ///< file already open
    InvalidFilename, ///< invalid filename
    InvalidData, ///< invalid data
    InvalidParam, ///< invalid param
    OpenError, ///< open error
    StatError, ///< get stat error
    NotOpen, ///< not open yet
    NoMem, ///< no memory
    ReadError, ///< read error
    Cancelled, ///< cancelled
    Busy, ///< busy
    Interrupt, ///< interrupt requested
    SeekError, ///< seek error
    FileChanged, ///< file changed
    Exception,
    CannotGetDbConnection, ///< DEPRECATED
    DatabaseQueryError, ///< DEPRECATED
    UserIdNotFound,
    NotAdmin,
};
namespace proto
{
namespace code
{
/// [0, 999]
enum Generic {
  Ok                 = 0,
  Valid              = 1,
  PermissionDenied   = 2,
  Invalid            = 3,
  Exception          = 999,
};
enum Database {
  // [20000, 20199]: database + group code
  NoSuchMember       = 20000,
  NoSuchGroup        = 20001,
  GroupDeleted       = 20002,
  GroupInfoNotChange = 20003,
  InvalidGroupInfoOp = 20004,
  InvalidGroupMemOp  = 20005,
  // Other database error: [20200, 20499]
  QueryError         = 20200,
  NoChange           = 20201,
  NoConnection       = 20202,
  ExecuteError       = 20203,
};
}
}
static constexpr inline int32_t GetErrno(Errno const& e) noexcept
{
    return static_cast<int32_t>(e);
}
constexpr int32_t kCppbaseErrnoOffset = 100000;
static constexpr inline int32_t GetErrno(int const e) noexcept
{
    return static_cast<int32_t>(e) + kCppbaseErrnoOffset;
}
extern std::string ErrnoToString(Errno const& e) noexcept;
static inline std::string ErrnoToString(int32_t const e) noexcept
{
    return ErrnoToString(static_cast<Errno>(e));
}
inline std::ostream& operator<<(std::ostream& os, Errno const& e) noexcept
{
    return os << ErrnoToString(e);
}
}
