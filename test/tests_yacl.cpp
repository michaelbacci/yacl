#include "tests_yacl.hpp"
#include "yacl.hpp"

namespace yacl {
namespace test {

void tests_yacl::SetUp() {
  ::testing::Test::SetUp();
}

TEST_F(tests_yacl, map_simple_values) {
  yacl::map map;

  map["host"].req<std::string>("h", "the remote host name");
  map["port"].opt<int>("p", "the remote host port", 80);

  ASSERT_EQ(map["host"].help(), "the remote host name");
//  ASSERT_EQ(map["port"].help(), "the remote host port");
//  ASSERT_EQ(map.size(), 2);

//  ASSERT_TRUE(map.parse("--host=http://github.com --port=23"));

//  ASSERT_EQ(map["host"].get(), "http://github.com");
//  ASSERT_EQ(map["host"].get<std::string>(), "http://github.com");
//  ASSERT_EQ(map["port"].get<int>(), 80);
}

TEST_F(tests_yacl, map_empty_assert) {
  yacl::map map;

  ASSERT_THROW(map.help(), std::domain_error);
  ASSERT_THROW(map.get(), std::domain_error);
  ASSERT_THROW(map.get<int>(), std::domain_error);
  ASSERT_THROW(map.req<int>("x", "x"), std::domain_error);
  ASSERT_THROW(map.opt<int>("y", "y", 0), std::domain_error);
}

}
}
