/*  Copyright (C) 2014-2021 FastoGT. All right reserved.

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

#pragma once

#include <stdlib.h>
#include <string.h>

#include <common/uri/url_parse.h>

namespace common {
namespace uri {

// Canonicalizer output -------------------------------------------------------

// Base class for the canonicalizer output, this maintains a buffer and
// supports simple resizing and append operations on it.
//
// It is VERY IMPORTANT that no virtual function calls be made on the common
// code path. We only have two virtual function calls, the destructor and a
// resize function that is called when the existing buffer is not big enough.
// The derived class is then in charge of setting up our buffer which we will
// manage.
template <typename T>
class CanonOutputT {
 public:
  CanonOutputT() : buffer_(NULL), buffer_len_(0), cur_len_(0) {}
  virtual ~CanonOutputT() {}

  // Implemented to resize the buffer. This function should update the buffer
  // pointer to point to the new buffer, and any old data up to |cur_len_| in
  // the buffer must be copied over.
  //
  // The new size |sz| must be larger than buffer_len_.
  virtual void Resize(int sz) = 0;

  // Accessor for returning a character at a given position. The input offset
  // must be in the valid range.
  inline T at(int offset) const { return buffer_[offset]; }

  // Sets the character at the given position. The given position MUST be less
  // than the length().
  inline void set(int offset, T ch) { buffer_[offset] = ch; }

  // Returns the number of characters currently in the buffer.
  inline int length() const { return cur_len_; }

  // Returns the current capacity of the buffer. The length() is the number of
  // characters that have been declared to be written, but the capacity() is
  // the number that can be written without reallocation. If the caller must
  // write many characters at once, it can make sure there is enough capacity,
  // write the data, then use set_size() to declare the new length().
  int capacity() const { return buffer_len_; }

  // Called by the user of this class to get the output. The output will NOT
  // be NULL-terminated. Call length() to get the
  // length.
  const T* data() const { return buffer_; }
  T* data() { return buffer_; }

  // Shortens the URL to the new length. Used for "backing up" when processing
  // relative paths. This can also be used if an external function writes a lot
  // of data to the buffer (when using the "Raw" version below) beyond the end,
  // to declare the new length.
  //
  // This MUST NOT be used to expand the size of the buffer beyond capacity().
  void set_length(int new_len) { cur_len_ = new_len; }

  // This is the most performance critical function, since it is called for
  // every character.
  void push_back(T ch) {
    // In VC2005, putting this common case first speeds up execution
    // dramatically because this branch is predicted as taken.
    if (cur_len_ < buffer_len_) {
      buffer_[cur_len_] = ch;
      cur_len_++;
      return;
    }

    // Grow the buffer to hold at least one more item. Hopefully we won't have
    // to do this very often.
    if (!Grow(1))
      return;

    // Actually do the insertion.
    buffer_[cur_len_] = ch;
    cur_len_++;
  }

  // Appends the given string to the output.
  void Append(const T* str, int str_len) {
    if (cur_len_ + str_len > buffer_len_) {
      if (!Grow(cur_len_ + str_len - buffer_len_))
        return;
    }
    for (int i = 0; i < str_len; i++)
      buffer_[cur_len_ + i] = str[i];
    cur_len_ += str_len;
  }

  void ReserveSizeIfNeeded(int estimated_size) {
    // Reserve a bit extra to account for escaped chars.
    if (estimated_size > buffer_len_)
      Resize(estimated_size + 8);
  }

 protected:
  // Grows the given buffer so that it can fit at least |min_additional|
  // characters. Returns true if the buffer could be resized, false on OOM.
  bool Grow(int min_additional) {
    static const int kMinBufferLen = 16;
    int new_len = (buffer_len_ == 0) ? kMinBufferLen : buffer_len_;
    do {
      if (new_len >= (1 << 30))  // Prevent overflow below.
        return false;
      new_len *= 2;
    } while (new_len < buffer_len_ + min_additional);
    Resize(new_len);
    return true;
  }

  T* buffer_;
  int buffer_len_;

  // Used characters in the buffer.
  int cur_len_;
};

// Simple implementation of the CanonOutput using new[]. This class
// also supports a static buffer so if it is allocated on the stack, most
// URLs can be canonicalized with no heap allocations.
template <typename T, int fixed_capacity = 1024>
class RawCanonOutputT : public CanonOutputT<T> {
 public:
  RawCanonOutputT() : CanonOutputT<T>() {
    this->buffer_ = fixed_buffer_;
    this->buffer_len_ = fixed_capacity;
  }
  ~RawCanonOutputT() override {
    if (this->buffer_ != fixed_buffer_)
      delete[] this->buffer_;
  }

  void Resize(int sz) override {
    T* new_buf = new T[sz];
    memcpy(new_buf, this->buffer_, sizeof(T) * (this->cur_len_ < sz ? this->cur_len_ : sz));
    if (this->buffer_ != fixed_buffer_)
      delete[] this->buffer_;
    this->buffer_ = new_buf;
    this->buffer_len_ = sz;
  }

 protected:
  T fixed_buffer_[fixed_capacity];
};

// Explicitely instantiate commonly used instatiations.
extern template class CanonOutputT<char>;
extern template class CanonOutputT<char16>;

// Normally, all canonicalization output is in narrow characters. We support
// the templates so it can also be used internally if a wide buffer is
// required.
typedef CanonOutputT<char> CanonOutput;
typedef CanonOutputT<char16> CanonOutputW;

template <int fixed_capacity>
class RawCanonOutput : public RawCanonOutputT<char, fixed_capacity> {};
template <int fixed_capacity>
class RawCanonOutputW : public RawCanonOutputT<char16, fixed_capacity> {};

// Character set converter ----------------------------------------------------
//
// Converts query strings into a custom encoding. The embedder can supply an
// implementation of this class to interface with their own character set
// conversion libraries.
//
// Embedders will want to see the unit test for the ICU version.

class CharsetConverter {
 public:
  CharsetConverter() {}
  virtual ~CharsetConverter() {}

  // Converts the given input string from UTF-16 to whatever output format the
  // converter supports. This is used only for the query encoding conversion,
  // which does not fail. Instead, the converter should insert "invalid
  // character" characters in the output for invalid sequences, and do the
  // best it can.
  //
  // If the input contains a character not representable in the output
  // character set, the converter should append the HTML entity sequence in
  // decimal, (such as "&#20320;") with escaping of the ampersand, number
  // sign, and semicolon (in the previous example it would be
  // "%26%2320320%3B"). This rule is based on what IE does in this situation.
  virtual void ConvertFromUTF16(const char16* input, int input_len, CanonOutput* output) = 0;
};

// Schemes --------------------------------------------------------------------

// Types of a scheme representing the requirements on the data represented by
// the authority component of a URL with the scheme.
enum SchemeType {
  // The authority component of a URL with the scheme has the form
  // "username:password@host:port". The username and password entries are
  // optional; the host may not be empty. The default value of the port can be
  // omitted in serialization. This type occurs with network schemes like http,
  // https, and ftp.
  SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION,
  // The authority component of a URL with the scheme has the form "host:port",
  // and does not include username or password. The default value of the port
  // can be omitted in serialization. Used by inner URLs of filesystem URLs of
  // origins with network hosts, from which the username and password are
  // stripped.
  SCHEME_WITH_HOST_AND_PORT,
  // The authority component of an URL with the scheme has the form "host", and
  // does not include port, username, or password. Used when the hosts are not
  // network addresses; for example, schemes used internally by the browser.
  SCHEME_WITH_HOST,
  // A URL with the scheme doesn't have the authority component.
  SCHEME_WITHOUT_AUTHORITY,
};

// Whitespace -----------------------------------------------------------------

// Searches for whitespace that should be removed from the middle of URLs, and
// removes it. Removed whitespace are tabs and newlines, but NOT spaces. Spaces
// are preserved, which is what most browsers do. A pointer to the output will
// be returned, and the length of that output will be in |output_len|.
//
// This should be called before parsing if whitespace removal is desired (which
// it normally is when you are canonicalizing).
//
// If no whitespace is removed, this function will not use the buffer and will
// return a pointer to the input, to avoid the extra copy. If modification is
// required, the given |buffer| will be used and the returned pointer will
// point to the beginning of the buffer.
//
// Therefore, callers should not use the buffer, since it may actually be empty,
// use the computed pointer and |*output_len| instead.
//
// If |input| contained both removable whitespace and a raw `<` character,
// |potentially_dangling_markup| will be set to `true`. Otherwise, it will be
// left untouched.

const char* RemoveURLWhitespace(const char* input,
                                int input_len,
                                CanonOutputT<char>* buffer,
                                int* output_len,
                                bool* potentially_dangling_markup);

const char16* RemoveURLWhitespace(const char16* input,
                                  int input_len,
                                  CanonOutputT<char16>* buffer,
                                  int* output_len,
                                  bool* potentially_dangling_markup);

// IDN ------------------------------------------------------------------------

// Converts the Unicode input representing a hostname to ASCII using IDN rules.
// The output must fall in the ASCII range, but will be encoded in UTF-16.
//
// On success, the output will be filled with the ASCII host name and it will
// return true. Unlike most other canonicalization functions, this assumes that
// the output is empty. The beginning of the host will be at offset 0, and
// the length of the output will be set to the length of the new host name.
//
// On error, returns false. The output in this case is undefined.

bool IDNToASCII(const char16* src, int src_len, CanonOutputW* output);

// Piece-by-piece canonicalizers ----------------------------------------------
//
// These individual canonicalizers append the canonicalized versions of the
// corresponding URL component to the given std::string. The spec and the
// previously-identified range of that component are the input. The range of
// the canonicalized component will be written to the output component.
//
// These functions all append to the output so they can be chained. Make sure
// the output is empty when you start.
//
// These functions returns boolean values indicating success. On failure, they
// will attempt to write something reasonable to the output so that, if
// displayed to the user, they will recognise it as something that's messed up.
// Nothing more should ever be done with these invalid URLs, however.

// Scheme: Appends the scheme and colon to the URL. The output component will
// indicate the range of characters up to but not including the colon.
//
// Canonical URLs always have a scheme. If the scheme is not present in the
// input, this will just write the colon to indicate an empty scheme. Does not
// append slashes which will be needed before any authority components for most
// URLs.
//
// The 8-bit version requires UTF-8 encoding.

bool CanonicalizeScheme(const char* spec, const Component& scheme, CanonOutput* output, Component* out_scheme);

bool CanonicalizeScheme(const char16* spec, const Component& scheme, CanonOutput* output, Component* out_scheme);

// User info: username/password. If present, this will add the delimiters so
// the output will be "<username>:<password>@" or "<username>@". Empty
// username/password pairs, or empty passwords, will get converted to
// nonexistent in the canonical version.
//
// The components for the username and password refer to ranges in the
// respective source strings. Usually, these will be the same string, which
// is legal as long as the two components don't overlap.
//
// The 8-bit version requires UTF-8 encoding.

bool CanonicalizeUserInfo(const char* username_source,
                          const Component& username,
                          const char* password_source,
                          const Component& password,
                          CanonOutput* output,
                          Component* out_username,
                          Component* out_password);

bool CanonicalizeUserInfo(const char16* username_source,
                          const Component& username,
                          const char16* password_source,
                          const Component& password,
                          CanonOutput* output,
                          Component* out_username,
                          Component* out_password);

// This structure holds detailed state exported from the IP/Host canonicalizers.
// Additional fields may be added as callers require them.
struct CanonHostInfo {
  CanonHostInfo() : family(NEUTRAL), num_ipv4_components(0), out_host() {}

  // Convenience function to test if family is an IP address.
  bool IsIPAddress() const { return family == IPV4 || family == IPV6; }

  // This field summarizes how the input was classified by the canonicalizer.
  enum Family {
    NEUTRAL,  // - Doesn't resemble an IP address. As far as the IP
              //   canonicalizer is concerned, it should be treated as a
              //   hostname.
    BROKEN,   // - Almost an IP, but was not canonicalized. This could be an
              //   IPv4 address where truncation occurred, or something
              //   containing the special characters :[] which did not parse
              //   as an IPv6 address. Never attempt to connect to this
              //   address, because it might actually succeed!
    IPV4,     // - Successfully canonicalized as an IPv4 address.
    IPV6,     // - Successfully canonicalized as an IPv6 address.
  };
  Family family;

  // If |family| is IPV4, then this is the number of nonempty dot-separated
  // components in the input text, from 1 to 4. If |family| is not IPV4,
  // this value is undefined.
  int num_ipv4_components;

  // Location of host within the canonicalized output.
  // CanonicalizeIPAddress() only sets this field if |family| is IPV4 or IPV6.
  // CanonicalizeHostVerbose() always sets it.
  Component out_host;

  // |address| contains the parsed IP Address (if any) in its first
  // AddressLength() bytes, in network order. If IsIPAddress() is false
  // AddressLength() will return zero and the content of |address| is undefined.
  unsigned char address[16];

  // Convenience function to calculate the length of an IP address corresponding
  // to the current IP version in |family|, if any. For use with |address|.
  int AddressLength() const { return family == IPV4 ? 4 : (family == IPV6 ? 16 : 0); }
};

// Host.
//
// The 8-bit version requires UTF-8 encoding. Use this version when you only
// need to know whether canonicalization succeeded.

bool CanonicalizeHost(const char* spec, const Component& host, CanonOutput* output, Component* out_host);

bool CanonicalizeHost(const char16* spec, const Component& host, CanonOutput* output, Component* out_host);

// Extended version of CanonicalizeHost, which returns additional information.
// Use this when you need to know whether the hostname was an IP address.
// A successful return is indicated by host_info->family != BROKEN. See the
// definition of CanonHostInfo above for details.

void CanonicalizeHostVerbose(const char* spec, const Component& host, CanonOutput* output, CanonHostInfo* host_info);

void CanonicalizeHostVerbose(const char16* spec, const Component& host, CanonOutput* output, CanonHostInfo* host_info);

// Canonicalizes a string according to the host canonicalization rules. Unlike
// CanonicalizeHost, this will not check for IP addresses which can change the
// meaning (and canonicalization) of the components. This means it is possible
// to call this for sub-components of a host name without corruption.
//
// As an example, "01.02.03.04.com" is a canonical hostname. If you called
// CanonicalizeHost on the substring "01.02.03.04" it will get "fixed" to
// "1.2.3.4" which will produce an invalid host name when reassembled. This
// can happen more than one might think because all numbers by themselves are
// considered IP addresses; so "5" canonicalizes to "0.0.0.5".
//
// Be careful: Because Punycode works on each dot-separated substring as a
// unit, you should only pass this function substrings that represent complete
// dot-separated subcomponents of the original host. Even if you have ASCII
// input, percent-escaped characters will have different meanings if split in
// the middle.
//
// Returns true if the host was valid. This function will treat a 0-length
// host as valid (because it's designed to be used for substrings) while the
// full version above will mark empty hosts as broken.

bool CanonicalizeHostSubstring(const char* spec, const Component& host, CanonOutput* output);

bool CanonicalizeHostSubstring(const char16* spec, const Component& host, CanonOutput* output);

// IP addresses.
//
// Tries to interpret the given host name as an IPv4 or IPv6 address. If it is
// an IP address, it will canonicalize it as such, appending it to |output|.
// Additional status information is returned via the |*host_info| parameter.
// See the definition of CanonHostInfo above for details.
//
// This is called AUTOMATICALLY from the host canonicalizer, which ensures that
// the input is unescaped and name-prepped, etc. It should not normally be
// necessary or wise to call this directly.

void CanonicalizeIPAddress(const char* spec, const Component& host, CanonOutput* output, CanonHostInfo* host_info);

void CanonicalizeIPAddress(const char16* spec, const Component& host, CanonOutput* output, CanonHostInfo* host_info);

// Port: this function will add the colon for the port if a port is present.
// The caller can pass PORT_UNSPECIFIED as the
// default_port_for_scheme argument if there is no default port.
//
// The 8-bit version requires UTF-8 encoding.

bool CanonicalizePort(const char* spec,
                      const Component& port,
                      int default_port_for_scheme,
                      CanonOutput* output,
                      Component* out_port);

bool CanonicalizePort(const char16* spec,
                      const Component& port,
                      int default_port_for_scheme,
                      CanonOutput* output,
                      Component* out_port);

// Returns the default port for the given canonical scheme, or PORT_UNSPECIFIED
// if the scheme is unknown.

int DefaultPortForScheme(const char* scheme, int scheme_len);

// Path. If the input does not begin in a slash (including if the input is
// empty), we'll prepend a slash to the path to make it canonical.
//
// The 8-bit version assumes UTF-8 encoding, but does not verify the validity
// of the UTF-8 (i.e., you can have invalid UTF-8 sequences, invalid
// characters, etc.). Normally, URLs will come in as UTF-16, so this isn't
// an issue. Somebody giving us an 8-bit path is responsible for generating
// the path that the server expects (we'll escape high-bit characters), so
// if something is invalid, it's their problem.

bool CanonicalizePath(const char* spec, const Component& path, CanonOutput* output, Component* out_path);

bool CanonicalizePath(const char16* spec, const Component& path, CanonOutput* output, Component* out_path);

// Canonicalizes the input as a file path. This is like CanonicalizePath except
// that it also handles Windows drive specs. For example, the path can begin
// with "c|\" and it will get properly canonicalized to "C:/".
// The string will be appended to |*output| and |*out_path| will be updated.
//
// The 8-bit version requires UTF-8 encoding.

bool FileCanonicalizePath(const char* spec, const Component& path, CanonOutput* output, Component* out_path);

bool FileCanonicalizePath(const char16* spec, const Component& path, CanonOutput* output, Component* out_path);

// Query: Prepends the ? if needed.
//
// The 8-bit version requires the input to be UTF-8 encoding. Incorrectly
// encoded characters (in UTF-8 or UTF-16) will be replaced with the Unicode
// "invalid character." This function can not fail, we always just try to do
// our best for crazy input here since web pages can set it themselves.
//
// This will convert the given input into the output encoding that the given
// character set converter object provides. The converter will only be called
// if necessary, for ASCII input, no conversions are necessary.
//
// The converter can be NULL. In this case, the output encoding will be UTF-8.

void CanonicalizeQuery(const char* spec,
                       const Component& query,
                       CharsetConverter* converter,
                       CanonOutput* output,
                       Component* out_query);

void CanonicalizeQuery(const char16* spec,
                       const Component& query,
                       CharsetConverter* converter,
                       CanonOutput* output,
                       Component* out_query);

// Ref: Prepends the # if needed. The output will be UTF-8 (this is the only
// canonicalizer that does not produce ASCII output). The output is
// guaranteed to be valid UTF-8.
//
// This function will not fail. If the input is invalid UTF-8/UTF-16, we'll use
// the "Unicode replacement character" for the confusing bits and copy the rest.

void CanonicalizeRef(const char* spec, const Component& path, CanonOutput* output, Component* out_path);

void CanonicalizeRef(const char16* spec, const Component& path, CanonOutput* output, Component* out_path);

// Full canonicalizer ---------------------------------------------------------
//
// These functions replace any string contents, rather than append as above.
// See the above piece-by-piece functions for information specific to
// canonicalizing individual components.
//
// The output will be ASCII except the reference fragment, which may be UTF-8.
//
// The 8-bit versions require UTF-8 encoding.

// Use for standard URLs with authorities and paths.

bool CanonicalizeStandardURL(const char* spec,
                             int spec_len,
                             const Parsed& parsed,
                             SchemeType scheme_type,
                             CharsetConverter* query_converter,
                             CanonOutput* output,
                             Parsed* new_parsed);

bool CanonicalizeStandardURL(const char16* spec,
                             int spec_len,
                             const Parsed& parsed,
                             SchemeType scheme_type,
                             CharsetConverter* query_converter,
                             CanonOutput* output,
                             Parsed* new_parsed);

// Use for file URLs.

bool CanonicalizeFileURL(const char* spec,
                         int spec_len,
                         const Parsed& parsed,
                         CharsetConverter* query_converter,
                         CanonOutput* output,
                         Parsed* new_parsed);

bool CanonicalizeFileURL(const char16* spec,
                         int spec_len,
                         const Parsed& parsed,
                         CharsetConverter* query_converter,
                         CanonOutput* output,
                         Parsed* new_parsed);

// Use for dev URLs.

bool CanonicalizeDevURL(const char* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

bool CanonicalizeDevURL(const char16* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

// Use for udp URLs.
bool CanonicalizeUdpURL(const char* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

bool CanonicalizeUdpURL(const char16* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

// Use for rtp URLs.
bool CanonicalizeRtpURL(const char* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

bool CanonicalizeRtpURL(const char16* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

// Use for srt URLs.
bool CanonicalizeSrtURL(const char* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

bool CanonicalizeSrtURL(const char16* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

// Use for tcp URLs.
bool CanonicalizeTcpURL(const char* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

bool CanonicalizeTcpURL(const char16* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

// Use for rtmp URLs.
bool CanonicalizeRtmpURL(const char* spec,
                         int spec_len,
                         const Parsed& parsed,
                         CharsetConverter* query_converter,
                         CanonOutput* output,
                         Parsed* new_parsed);

bool CanonicalizeRtmpURL(const char16* spec,
                         int spec_len,
                         const Parsed& parsed,
                         CharsetConverter* query_converter,
                         CanonOutput* output,
                         Parsed* new_parsed);

// Use for rtsp URLs.
bool CanonicalizeRtspURL(const char* spec,
                         int spec_len,
                         const Parsed& parsed,
                         CharsetConverter* query_converter,
                         CanonOutput* output,
                         Parsed* new_parsed);

bool CanonicalizeRtspURL(const char16* spec,
                         int spec_len,
                         const Parsed& parsed,
                         CharsetConverter* query_converter,
                         CanonOutput* output,
                         Parsed* new_parsed);

// Use for path URLs such as javascript. This does not modify the path in any
// way, for example, by escaping it.

bool CanonicalizePathURL(const char* spec, int spec_len, const Parsed& parsed, CanonOutput* output, Parsed* new_parsed);

bool CanonicalizePathURL(const char16* spec,
                         int spec_len,
                         const Parsed& parsed,
                         CanonOutput* output,
                         Parsed* new_parsed);

// Part replacer --------------------------------------------------------------

// Internal structure used for storing separate strings for each component.
// The basic canonicalization functions use this structure internally so that
// component replacement (different strings for different components) can be
// treated on the same code path as regular canonicalization (the same string
// for each component).
//
// A Parsed structure usually goes along with this. Those components identify
// offsets within these strings, so that they can all be in the same string,
// or spread arbitrarily across different ones.
//
// This structures does not own any data. It is the caller's responsibility to
// ensure that the data the pointers point to stays in scope and is not
// modified.
template <typename CHAR>
struct URLComponentSource {
  // Constructor normally used by callers wishing to replace components. This
  // will make them all NULL, which is no replacement. The caller would then
  // override the components they want to replace.
  URLComponentSource()
      : scheme(NULL), username(NULL), password(NULL), host(NULL), port(NULL), path(NULL), query(NULL), ref(NULL) {}

  // Constructor normally used internally to initialize all the components to
  // point to the same spec.
  explicit URLComponentSource(const CHAR* default_value)
      : scheme(default_value),
        username(default_value),
        password(default_value),
        host(default_value),
        port(default_value),
        path(default_value),
        query(default_value),
        ref(default_value) {}

  const CHAR* scheme;
  const CHAR* username;
  const CHAR* password;
  const CHAR* host;
  const CHAR* port;
  const CHAR* path;
  const CHAR* query;
  const CHAR* ref;
};

// This structure encapsulates information on modifying a URL. Each component
// may either be left unchanged, replaced, or deleted.
//
// By default, each component is unchanged. For those components that should be
// modified, call either Set* or Clear* to modify it.
//
// The string passed to Set* functions DOES NOT GET COPIED AND MUST BE KEPT
// IN SCOPE BY THE CALLER for as long as this object exists!
//
// Prefer the 8-bit replacement version if possible since it is more efficient.
template <typename CHAR>
class Replacements {
 public:
  Replacements() {}

  // Scheme
  void SetScheme(const CHAR* s, const Component& comp) {
    sources_.scheme = s;
    components_.scheme = comp;
  }
  // Note: we don't have a ClearScheme since this doesn't make any sense.
  bool IsSchemeOverridden() const { return sources_.scheme != NULL; }

  // Username
  void SetUsername(const CHAR* s, const Component& comp) {
    sources_.username = s;
    components_.username = comp;
  }
  void ClearUsername() {
    sources_.username = Placeholder();
    components_.username = Component();
  }
  bool IsUsernameOverridden() const { return sources_.username != NULL; }

  // Password
  void SetPassword(const CHAR* s, const Component& comp) {
    sources_.password = s;
    components_.password = comp;
  }
  void ClearPassword() {
    sources_.password = Placeholder();
    components_.password = Component();
  }
  bool IsPasswordOverridden() const { return sources_.password != NULL; }

  // Host
  void SetHost(const CHAR* s, const Component& comp) {
    sources_.host = s;
    components_.host = comp;
  }
  void ClearHost() {
    sources_.host = Placeholder();
    components_.host = Component();
  }
  bool IsHostOverridden() const { return sources_.host != NULL; }

  // Port
  void SetPort(const CHAR* s, const Component& comp) {
    sources_.port = s;
    components_.port = comp;
  }
  void ClearPort() {
    sources_.port = Placeholder();
    components_.port = Component();
  }
  bool IsPortOverridden() const { return sources_.port != NULL; }

  // Path
  void SetPath(const CHAR* s, const Component& comp) {
    sources_.path = s;
    components_.path = comp;
  }
  void ClearPath() {
    sources_.path = Placeholder();
    components_.path = Component();
  }
  bool IsPathOverridden() const { return sources_.path != NULL; }

  // Query
  void SetQuery(const CHAR* s, const Component& comp) {
    sources_.query = s;
    components_.query = comp;
  }
  void ClearQuery() {
    sources_.query = Placeholder();
    components_.query = Component();
  }
  bool IsQueryOverridden() const { return sources_.query != NULL; }

  // Ref
  void SetRef(const CHAR* s, const Component& comp) {
    sources_.ref = s;
    components_.ref = comp;
  }
  void ClearRef() {
    sources_.ref = Placeholder();
    components_.ref = Component();
  }
  bool IsRefOverridden() const { return sources_.ref != NULL; }

  // Getters for the internal data. See the variables below for how the
  // information is encoded.
  const URLComponentSource<CHAR>& sources() const { return sources_; }
  const Parsed& components() const { return components_; }

 private:
  // Returns a pointer to a static empty string that is used as a placeholder
  // to indicate a component should be deleted (see below).
  const CHAR* Placeholder() {
    static const CHAR empty_cstr = 0;
    return &empty_cstr;
  }

  // We support three states:
  //
  // Action                 | Source                Component
  // -----------------------+--------------------------------------------------
  // Don't change component | NULL                  (unused)
  // Replace component      | (replacement string)  (replacement component)
  // Delete component       | (non-NULL)            (invalid component: (0,-1))
  //
  // We use a pointer to the empty string for the source when the component
  // should be deleted.
  URLComponentSource<CHAR> sources_;
  Parsed components_;
};

// The base must be an 8-bit canonical URL.

bool ReplaceStandardURL(const char* base,
                        const Parsed& base_parsed,
                        const Replacements<char>& replacements,
                        SchemeType scheme_type,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

bool ReplaceStandardURL(const char* base,
                        const Parsed& base_parsed,
                        const Replacements<char16>& replacements,
                        SchemeType scheme_type,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed);

// Replacing some parts of a file URL is not permitted. Everything except
// the host, path, query, and ref will be ignored.

bool ReplaceFileURL(const char* base,
                    const Parsed& base_parsed,
                    const Replacements<char>& replacements,
                    CharsetConverter* query_converter,
                    CanonOutput* output,
                    Parsed* new_parsed);

bool ReplaceFileURL(const char* base,
                    const Parsed& base_parsed,
                    const Replacements<char16>& replacements,
                    CharsetConverter* query_converter,
                    CanonOutput* output,
                    Parsed* new_parsed);

// Path URLs can only have the scheme and path replaced. All other components
// will be ignored.

bool ReplacePathURL(const char* base,
                    const Parsed& base_parsed,
                    const Replacements<char>& replacements,
                    CanonOutput* output,
                    Parsed* new_parsed);

bool ReplacePathURL(const char* base,
                    const Parsed& base_parsed,
                    const Replacements<char16>& replacements,
                    CanonOutput* output,
                    Parsed* new_parsed);

// Relative URL ---------------------------------------------------------------

// Given an input URL or URL fragment |fragment|, determines if it is a
// relative or absolute URL and places the result into |*is_relative|. If it is
// relative, the relevant portion of the URL will be placed into
// |*relative_component| (there may have been trimmed whitespace, for example).
// This value is passed to ResolveRelativeURL. If the input is not relative,
// this value is UNDEFINED (it may be changed by the function).
//
// Returns true on success (we successfully determined the URL is relative or
// not). Failure means that the combination of URLs doesn't make any sense.
//
// The base URL should always be canonical, therefore is ASCII.

bool IsRelativeURL(const char* base,
                   const Parsed& base_parsed,
                   const char* fragment,
                   int fragment_len,
                   bool is_base_hierarchical,
                   bool* is_relative,
                   Component* relative_component);

bool IsRelativeURL(const char* base,
                   const Parsed& base_parsed,
                   const char16* fragment,
                   int fragment_len,
                   bool is_base_hierarchical,
                   bool* is_relative,
                   Component* relative_component);

// Given a canonical parsed source URL, a URL fragment known to be relative,
// and the identified relevant portion of the relative URL (computed by
// IsRelativeURL), this produces a new parsed canonical URL in |output| and
// |out_parsed|.
//
// It also requires a flag indicating whether the base URL is a file: URL
// which triggers additional logic.
//
// The base URL should be canonical and have a host (may be empty for file
// URLs) and a path. If it doesn't have these, we can't resolve relative
// URLs off of it and will return the base as the output with an error flag.
// Because it is canonical is should also be ASCII.
//
// The query charset converter follows the same rules as CanonicalizeQuery.
//
// Returns true on success. On failure, the output will be "something
// reasonable" that will be consistent and valid, just probably not what
// was intended by the web page author or caller.

bool ResolveRelativeURL(const char* base_url,
                        const Parsed& base_parsed,
                        bool base_is_file,
                        const char* relative_url,
                        const Component& relative_component,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* out_parsed);

bool ResolveRelativeURL(const char* base_url,
                        const Parsed& base_parsed,
                        bool base_is_file,
                        const char16* relative_url,
                        const Component& relative_component,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* out_parsed);

}  // namespace uri
}  // namespace common
