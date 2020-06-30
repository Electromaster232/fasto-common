/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
        * Neither the name of FastoGT. nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <common/draw/point_conversions.h>

#include <common/convert2string.h>
#include <common/draw/safe_integer_conversions.h>
#include <common/sprintf.h>

namespace common {
namespace draw {

Point ToFlooredPoint(const PointF& point) {
  int x = ToFlooredInt(point.x());
  int y = ToFlooredInt(point.y());
  return Point(x, y);
}

Point ToCeiledPoint(const PointF& point) {
  int x = ToCeiledInt(point.x());
  int y = ToCeiledInt(point.y());
  return Point(x, y);
}

Point ToRoundedPoint(const PointF& point) {
  int x = ToRoundedInt(point.x());
  int y = ToRoundedInt(point.y());
  return Point(x, y);
}

std::ostream& operator<<(std::ostream& out, const PointF& point) {
  return out << point.ToString();
}

}  // namespace draw

std::string ConvertToString(const draw::PointF& value) {
  return MemSPrintf("%f,%f", value.x(), value.y());
}

bool ConvertFromString(const std::string& from, draw::PointF* out) {
  if (!out) {
    return false;
  }

  draw::PointF res;
  size_t del = from.find_first_of(',');
  if (del != std::string::npos) {
    float lx;
    if (!ConvertFromString(from.substr(0, del), &lx)) {
      return false;
    }
    res.set_x(lx);

    float ly;
    if (!ConvertFromString(from.substr(del + 1), &ly)) {
      return false;
    }
    res.set_y(ly);
  }

  *out = res;
  return true;
}
}  // namespace common
