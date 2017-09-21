#include "jwt/jwt_all.h"
#include "gtest/gtest.h"
#include <string>

class TokenTest : public ::testing::Test {
public:
  TokenTest()
      : validator_("secret"), lst_("sub", {"1234567890", "bar"}),
        validToken_("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
                    "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW"
                    "4iOnRydWV9."
                    "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQ") {}

  void BadHeader(std::string jwt) {
    ASSERT_THROW(JWT::Decode(jwt, &none_), TokenFormatError);
  }

  void ValidToken(std::string jwt, MessageValidator *validator,
                  ClaimValidator *claims) {
    jwt_ptr token(JWT::Decode(jwt, validator, claims));
    ASSERT_NE(nullptr, token);
  }

  void ImproperSignedToken(std::string jwt, MessageValidator *validator) {
    ASSERT_THROW(JWT::Decode(jwt, validator), InvalidTokenError);
  }

  void ImproperClaims(std::string jwt, ClaimValidator *claims) {
    ASSERT_THROW(JWT::Decode(jwt, nullptr, claims), InvalidClaimError);
  }

  NoneValidator none_;
  HS256Validator validator_;
  ListClaimValidator lst_;
  std::string validToken_;
};

TEST_F(TokenTest, bad_format_tokens) {
  BadHeader("eyJhbGciOiJIUzI1NiIsInR5cCI6IkJBUiJ9");
  BadHeader(
      "eyJhbGciOiJIUzI1NiIsInR5cCI6IkJBUiJ9."
      "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWV9");
  BadHeader(
      "e y J h b G c i O i J I U z I 1 N i I s I n R 5 c C I 6 I k p X V C J 9."
      "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWV9."
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQ");
  BadHeader("foo");
  BadHeader("......");

  // Bad JSON header
  BadHeader(
      "eyB7IGZvbyB9."
      "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWV9."
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQ");

  // Bad JSON payload
  BadHeader("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
            "eyB7IGZvbyB9."
            "eyB7IGZvbyB9");
}

TEST_F(TokenTest, valid_hs256) { ValidToken(validToken_, &validator_, &lst_); }

TEST_F(TokenTest, valid_hs256_claims) {
  ValidToken(validToken_, &validator_, &lst_);
}

TEST_F(TokenTest, encoded_token_is_valid) {
  ::json json = {{"sub", "1234567890"}, {"name", "John Doe"}, {"admin", true}};
  auto str_token = JWT::Encode(&validator_, json);
  jwt_ptr token(JWT::Decode(str_token, &validator_, &lst_));
  ValidToken(str_token, &validator_, &lst_);
}

TEST_F(TokenTest, modern_encoded_token_is_valid) {
  ::json json = {{"sub", "1234567890"}, {"name", "John Doe"}, {"admin", true}};
  std::string enc_token = JWT::Encode(&validator_, json, {});
  jwt_ptr token(JWT::Decode(enc_token, &validator_, &lst_));
  ValidToken(enc_token.c_str(), &validator_, &lst_);
}

TEST_F(TokenTest, encoded_token_with_none_is_valid) {
  NoneValidator none;
  ::json json = {{"sub", "1234567890"}, {"name", "John Doe"}, {"admin", true}};
  auto str_token = JWT::Encode(&none, json);
  ValidToken(str_token, &none, &lst_);
}

TEST_F(TokenTest, encoded_token_has_duplicates) {
  std::string token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJmb28iOiJiYXIifQ"
                      ".dtxWM6MIcgoeMgH87tGvsNDY6cHWL6MGW4LeYvnm1JA";
  ASSERT_THROW(JWT::Decode(token, &validator_, &lst_), InvalidTokenError);
}

TEST_F(TokenTest, token_with_large_wrong_signature) {

  std::string token =
      "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
      "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWV9."
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ"
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQTJVA95OrM7E2cBab30RMHrHDcEfxj"
      "oYZgeFONFh7HgQ";
  ASSERT_THROW(JWT::Decode(token, &validator_, &lst_), InvalidTokenError);
}

TEST_F(TokenTest, encoded_token_missing_alg) {
  std::string noAlg =
      "eyJmb28iOiJIUzI1NiJ9."
      "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWV9."
      "TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQ";
  ASSERT_THROW(JWT::Decode(noAlg, &validator_, &lst_), InvalidTokenError);
}

TEST_F(TokenTest, bad_alg) {
  std::string badAlg =
      "eyJhbGciOiJCTEEiLCJ0eXAiOiJKV1QifQ."
      "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWV9."
      "Pmjzsg4UPL6vKXK2pFGqH60qudcr4YHQ1e9Ddsl_ONo";
  ASSERT_THROW(JWT::Decode(badAlg, &validator_, &lst_), InvalidSignatureError);
}

TEST_F(TokenTest, encoded_token_has_custom_header) {
  ::json json = {{"sub", "1234567890"}, {"name", "John Doe"}, {"admin", true}};
  ::json header = {{"foo", "bar"}};

  auto tok = JWT::Encode(&validator_, json, header);
  jwt_ptr token(JWT::Decode(tok, &validator_, &lst_));
  ::json foo = token->header()["foo"];
  EXPECT_STREQ("bar", foo.get<std::string>().c_str());
}

/*
TEST_F(TokenTest, encoded_token_has_custom_header_with_double_entries) {
   ::json_ptr json(json_pack("{ss, ss, sb}", "sub", "1234567890", "name", "John
Doe", "admin", true));
   ::json_ptr header(json_pack("{ss, ss}", "foo", "bar", "foo", "baz"));

  str_ptr str_token(JWT::Encode(&validator_, json.get(), header.get()));
  jwt_ptr token(JWT::Decode(str_token.get(), &validator_, &lst_));
  EXPECT_NE(nullptr, token.get());
}
*/

TEST_F(TokenTest, just_parse) {
  jwt_ptr token(
      JWT::Decode(validToken_.c_str(), validToken_.size(), &validator_, &lst_));
  EXPECT_TRUE(token.get() != NULL);
}

TEST_F(TokenTest, parse_and_validate_bad_signature) {
  HS256Validator hs256("Not the right secret");
  ASSERT_THROW(JWT::Decode(validToken_, &hs256), InvalidSignatureError);
}

TEST_F(TokenTest, modified_token) {
  validToken_.append("A");
  ImproperSignedToken(validToken_, &validator_);
}

TEST_F(TokenTest, no_exp) {
  ExpValidator exp;
  ImproperClaims(validToken_, &exp);
}

TEST_F(TokenTest, payload_deserialize) {
  jwt_ptr token(JWT::Decode(validToken_));
  const json payload = token->payload();
  EXPECT_STREQ("1234567890", payload["sub"].get<std::string>().c_str());
  EXPECT_TRUE(payload["admin"].get<bool>());
  EXPECT_STREQ("John Doe", payload["name"].get<std::string>().c_str());
}
