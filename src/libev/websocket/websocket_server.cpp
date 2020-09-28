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

#include <common/libev/websocket/websocket_server.h>

#include <common/convert2string.h>
#include <common/hash/sha1.h>
#include <common/libev/websocket/websocket_client.h>
#include <common/utils.h>

namespace common {
namespace libev {
namespace websocket {

WebSocketServer::WebSocketServer(const net::HostAndPort& host, IoLoopObserver* observer) : TcpServer(host, observer) {}

const char* WebSocketServer::ClassName() const {
  return "WebSocketServer";
}

Error WebSocketServer::MakeSwitchProtocolsResponse(const std::string& key,
                                                   const std::vector<std::string>& protocols,
                                                   common::http::HttpResponse* resp) {
  if (key.empty() || protocols.empty() || !resp) {
    return make_error_inval();
  }

  common::http::HttpHeader upgrade("Upgrade", "websocket");
  common::http::HttpHeader user("User-Agent", USER_AGENT_VALUE);
  common::http::HttpHeader connection("Connection", "Upgrade");
  std::string raw_key = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

  common::hash::SHA1_CTX ctx;
  const common::buffer_t bytes_key = ConvertToBytes(raw_key);
  common::hash::SHA1_Init(&ctx);
  common::hash::SHA1_Update(&ctx, bytes_key.data(), bytes_key.size());
  unsigned char sha1_result[SHA1_HASH_LENGTH];
  common::hash::SHA1_Final(&ctx, sha1_result);
  std::string hexed;
  if (!common::utils::base64::encode64(MAKE_CHAR_BUFFER_SIZE(sha1_result, SHA1_HASH_LENGTH), &hexed)) {
    return make_error("can't encode key to base64");
  }

  common::http::HttpHeader sec_accept("Sec-WebSocket-Accept", hexed);
  std::string protocol_line = protocols[0];
  for (size_t i = 1; i < protocols.size(); ++i) {
    protocol_line += ", ";
    protocol_line += protocols[i];
  }
  common::http::HttpHeader sec_protocol("Sec-WebSocket-Protocol", protocol_line);
  *resp = common::http::HttpResponse(common::http::http_protocol::HP_1_1, common::http::http_status::HS_SWITCH_PROTOCOL,
                                     {upgrade, user, connection, sec_accept, sec_protocol}, char_buffer_t());
  return Error();
}

tcp::TcpClient* WebSocketServer::CreateClient(const net::socket_info& info) {
  return new WebSocketClient(this, info);
}

}  // namespace websocket
}  // namespace libev
}  // namespace common
