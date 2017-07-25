#include <gtest/gtest.h>

#include <common/text_decoders/base64_edcoder.h>
#include <common/text_decoders/compress_snappy_edcoder.h>
#include <common/text_decoders/compress_zlib_edcoder.h>
#include <common/text_decoders/hex_edcoder.h>
#include <common/text_decoders/html_edcoder.h>
#include <common/text_decoders/msgpack_edcoder.h>

TEST(msg_pack, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::MsgPackEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err && err->IsError());

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(raw_data, dec_data);
}

TEST(html, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::HtmlEscEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err && err->IsError());

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(raw_data, dec_data);
}

TEST(hex, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::HexEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err && err->IsError());

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(raw_data, dec_data);
}

TEST(base64, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::Base64EDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err && err->IsError());

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(raw_data, dec_data);
}

#ifdef HAVE_ZLIB
TEST(zlib, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::CompressZlibEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err && err->IsError());

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(raw_data, dec_data);
}
#endif

#ifdef HAVE_SNAPPY
TEST(snappy, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::CompressSnappyEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err && err->IsError());

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(raw_data, dec_data);
}
#endif