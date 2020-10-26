/*  Copyright (C) 2014-2020 FastoGT. All right reserved.
    This file is part of iptv_cloud.
    iptv_cloud is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    iptv_cloud is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with iptv_cloud.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common/serializer/json_serializer.h>

namespace common {
namespace {
const Error kInvalidType = Error("Invalid type");
const Error kNotExist = Error("Not exists field");
}  // namespace
namespace serializer {

Error json_get_string(json_object* json, const char* field, std::string* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jstring = nullptr;
  json_bool jstring_exists = json_object_object_get_ex(json, field, &jstring);
  if (!jstring_exists) {
    return kNotExist;
  }

  if (!json_object_is_type(jstring, json_type_string)) {
    return kInvalidType;
  }

  *out = json_object_get_string(jstring);
  return Error();
}

Error json_get_int(json_object* json, const char* field, int* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return kNotExist;
  }

  if (!json_object_is_type(jint, json_type_int)) {
    return kInvalidType;
  }

  *out = json_object_get_int(jint);
  return Error();
}

Error json_get_int64(json_object* json, const char* field, int64_t* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return kNotExist;
  }

  if (!json_object_is_type(jint, json_type_int)) {
    return kInvalidType;
  }

  *out = json_object_get_int64(jint);
  return Error();
}

Error json_get_uint64(json_object* json, const char* field, uint64_t* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return kNotExist;
  }

  if (!json_object_is_type(jint, json_type_int)) {
    return kInvalidType;
  }

  *out = json_object_get_uint64(jint);
  return Error();
}

Error json_get_double(json_object* json, const char* field, double* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return kNotExist;
  }

  if (!json_object_is_type(jint, json_type_double)) {
    return kInvalidType;
  }

  *out = json_object_get_double(jint);
  return Error();
}

Error json_get_bool(json_object* json, const char* field, bool* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return kNotExist;
  }

  if (!json_object_is_type(jint, json_type_boolean)) {
    return kInvalidType;
  }

  *out = json_object_get_boolean(jint);
  return Error();
}

Error json_get_array(json_object* json, const char* field, json_object** out, size_t* len) {
  if (!json || !field || !out || !len) {
    return make_error_inval();
  }

  json_object* jarray = nullptr;
  json_bool jarray_exists = json_object_object_get_ex(json, field, &jarray);
  if (!jarray_exists) {
    return kNotExist;
  }

  if (!json_object_is_type(jarray, json_type_array)) {
    return kInvalidType;
  }

  *out = jarray;
  *len = json_object_array_length(jarray);
  return Error();
}

Error json_get_object(json_object* json, const char* field, json_object** out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jobj = nullptr;
  json_bool jobj_exists = json_object_object_get_ex(json, field, &jobj);
  if (!jobj_exists) {
    return kNotExist;
  }

  if (!json_object_is_type(jobj, json_type_object)) {
    return kInvalidType;
  }

  *out = jobj;
  return Error();
}

}  // namespace serializer
}  // namespace common
