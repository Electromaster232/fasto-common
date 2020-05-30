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

#include <gtest/gtest.h>

#include <common/uri/gurl.h>

#define HTTP_PATH "/home/index.html"
#define FILE_PATH "/home/sasha/1.mp4"
#define DEV_VIDEO_PATH "/dev/video3"
#define UDP_LINK "udp://239.0.3.3:3003"
#define TCP_LINK "tcp://google.com:2121"
#define RTMP_LINK "rtmp://192.168.1.105:5423/live"
#define RTMP_LINK_DEFAULT "rtmp://a.rtmp.youtube.com/live2"
#define RTSP_LINK "rtsp://192.168.1.210:555/Streaming/Channels/101"
#define RTSP_LINK_USER "rtsp://root:password@192.168.1.111/axis-media/media.amp?videocodec=h264&audiocodec=aac"

#define HTTP_COMMON_LINK "http://www.permadi.com/index.html"
#define HTTP_PORT_LINK "http://111.119.160.90:81/fastocloud/hls/2/5ebeba5ba2ffe6cd5d4488d3/0/master.m3u8"
#define HTTP_LONG_LINK "http://www.permadi.com/tutorial/urlEncoding/index.html"
#define HTTPS_LINK "https://github.com/fastogt/fastotvlite_mobile"
#define HTTP_QUERY_LINK "http://www.permadi.com/tutorial/urlEncoding/example.html?var=This+is+a+simple+%26+short+test"

TEST(Url, IsValid) {
  common::uri::GURL invalid;
  ASSERT_FALSE(invalid.is_valid());

  common::uri::GURL http_common(HTTP_COMMON_LINK);
  ASSERT_TRUE(http_common.is_valid());
  ASSERT_TRUE(http_common.SchemeIsHTTPOrHTTPS());
  ASSERT_EQ(http_common.EffectiveIntPort(), 80);
  ASSERT_EQ(http_common.spec(), HTTP_COMMON_LINK);

  common::uri::GURL http_port(HTTP_PORT_LINK);
  ASSERT_TRUE(http_port.is_valid());
  ASSERT_TRUE(http_port.SchemeIsHTTPOrHTTPS());
  ASSERT_EQ(http_port.port(), "81");
  ASSERT_EQ(http_port.IntPort(), 81);
  ASSERT_EQ(http_port.spec(), HTTP_PORT_LINK);

  common::uri::GURL http_long(HTTP_LONG_LINK);
  ASSERT_TRUE(http_long.is_valid());
  ASSERT_TRUE(http_long.SchemeIsHTTPOrHTTPS());
  ASSERT_EQ(http_long.spec(), HTTP_LONG_LINK);

  common::uri::GURL https(HTTPS_LINK);
  ASSERT_TRUE(https.is_valid());
  ASSERT_TRUE(https.SchemeIsHTTPOrHTTPS());
  ASSERT_EQ(https.spec(), HTTPS_LINK);

  common::uri::GURL http_query(HTTP_QUERY_LINK);
  ASSERT_TRUE(http_query.is_valid());
  ASSERT_TRUE(http_query.SchemeIsHTTPOrHTTPS());
  ASSERT_EQ(http_query.spec(), HTTP_QUERY_LINK);

  const std::string originFile = "file://" + std::string(FILE_PATH);
  common::uri::GURL path5(originFile);
  ASSERT_TRUE(path5.is_valid());
  ASSERT_TRUE(path5.SchemeIsFile());
  ASSERT_EQ(FILE_PATH, path5.path());
  ASSERT_EQ(originFile, path5.spec());

  const std::string originDev = "dev://" + std::string(DEV_VIDEO_PATH);
  common::uri::GURL path6(originDev);
  ASSERT_TRUE(path6.is_valid());
  ASSERT_TRUE(path6.SchemeIsDev());
  ASSERT_EQ(DEV_VIDEO_PATH, path6.path());
  ASSERT_EQ(originDev, path6.spec());

  common::uri::GURL http_path(HTTP_PATH);
  ASSERT_FALSE(http_path.is_valid());

  common::uri::GURL udp(UDP_LINK);
  ASSERT_TRUE(udp.is_valid());
  ASSERT_TRUE(udp.SchemeIsUdp());
  ASSERT_EQ(udp.host(), "239.0.3.3");
  ASSERT_EQ(udp.port(), "3003");
  ASSERT_EQ(udp.spec(), UDP_LINK);

  common::uri::GURL tcp(TCP_LINK);
  ASSERT_TRUE(tcp.is_valid());
  ASSERT_TRUE(tcp.SchemeIsTcp());
  ASSERT_EQ(tcp.host(), "google.com");
  ASSERT_EQ(tcp.port(), "2121");
  ASSERT_EQ(tcp.spec(), TCP_LINK);

  common::uri::GURL rtmp(RTMP_LINK);
  ASSERT_TRUE(rtmp.is_valid());
  ASSERT_TRUE(rtmp.SchemeIsRtmp());
  ASSERT_EQ(rtmp.host(), "192.168.1.105");
  ASSERT_EQ(rtmp.port(), "5423");
  ASSERT_EQ(rtmp.path(), "/live");
  ASSERT_EQ(rtmp.spec(), RTMP_LINK);

  common::uri::GURL rtmp_default(RTMP_LINK_DEFAULT);
  ASSERT_TRUE(rtmp_default.is_valid());
  ASSERT_TRUE(rtmp_default.SchemeIsRtmp());
  ASSERT_EQ(rtmp_default.host(), "a.rtmp.youtube.com");
  ASSERT_EQ(rtmp_default.EffectiveIntPort(), 1935);
  ASSERT_EQ(rtmp_default.path(), "/live2");
  ASSERT_EQ(rtmp_default.spec(), RTMP_LINK_DEFAULT);

  common::uri::GURL rtsp(RTSP_LINK);
  ASSERT_TRUE(rtsp.is_valid());
  ASSERT_TRUE(rtsp.SchemeIsRtsp());
  ASSERT_EQ(rtsp.host(), "192.168.1.210");
  ASSERT_EQ(rtsp.port(), "555");
  ASSERT_EQ(rtsp.path(), "/Streaming/Channels/101");
  ASSERT_EQ(rtsp.spec(), RTSP_LINK);

  common::uri::GURL rtsp_user(RTSP_LINK_USER);
  ASSERT_TRUE(rtsp_user.is_valid());
  ASSERT_TRUE(rtsp_user.SchemeIsRtsp());
  ASSERT_EQ(rtsp_user.host(), "192.168.1.111");
  ASSERT_EQ(rtsp_user.EffectiveIntPort(), 554);
  ASSERT_EQ(rtsp_user.path(), "/axis-media/media.amp");
  ASSERT_EQ(rtsp_user.query(), "videocodec=h264&audiocodec=aac");
  ASSERT_EQ(rtsp_user.spec(), RTSP_LINK_USER);

  common::uri::GURL post("http://panel.fastotv.com:8083/panel_pro/api/load_balance/log/5ec602f6d392b6b89f1814f7");
  ASSERT_EQ(post.host(), "panel.fastotv.com");
  ASSERT_EQ(post.EffectiveIntPort(), 8083);
}
