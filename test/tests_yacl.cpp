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
  ASSERT_EQ(map.size(), 2);

  ASSERT_TRUE(map.parse("--host=http://github.com --port=23"));

//  ASSERT_EQ(map["host"].get(), "http://github.com");
//  ASSERT_EQ(map["host"].get<std::string>(), "http://github.com");
//  ASSERT_EQ(map["port"].get<int>(), 80);
}

TEST_F(tests_yacl, map_with_custom_checks) {
  struct one_or_two {
    int operator()(const std::string &s) {
      return (s == "1" || s == "2");
    }

  };

  yacl::map map;

  map["x"].req<int>("x", "x = {1,2,3,4,5}", yacl::oneof(1,2,3,4,5));
  map["x"].req<int>("x", "x = {1,2}", one_or_two());

  map["x"].req<std::string>("x", "x = {a,b}", yacl::oneof("a", "b"));
  map["x"].req<std::string>("x", "x = {a,b}", [](const std::string& x) { return x; });

  std::function<int(int)> f = [](int i) { return i; };
  map["x"].req<int>("x", "x = {1,2}",  f);

  map["x"].req<int>("x", "x = {1,2}", [&map](const std::string& x) {
    if (x != "0" || x != "1")
      throw std::domain_error(map["x"].help());
    return 0;
  });

  //TODO
  //map["x"].req<int>("x", "x = {1,2}", [](int i) { return i; });
  //or
  //map["x"].req<int,int>("x", "x = {1,2}", [](int i) { return i; });

}

TEST_F(tests_yacl, argc_argv_converter) {
  std::string s_argv = "--op1=123 positional_1 -v positional_2 -h --op2=str positional_3";

  int argc;
  char **argv;
  yacl::convert(s_argv) >> argc >> argv;

  ASSERT_EQ(argc, 7);
  ASSERT_EQ(std::string(argv[0]), "--op1=123");
  ASSERT_EQ(std::string(argv[1]), "positional_1");
  ASSERT_EQ(std::string(argv[6]), "positional_3");

  std::string s_convesion;
  yacl::convert(argc, argv) >> s_convesion;
  ASSERT_EQ(s_convesion, s_argv);
}

TEST_F(tests_yacl, simple_iterator) {

  std::string s_argv = "--op1=123 positional_1 -v positional_2 -h --op2=str positional_3" ;

//  for (auto arg : yacl::parse(argc,argv)) {
//
//  }
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
