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
  map["v"].opt<bool>("v", "the remote host port",false);

  ASSERT_EQ(map["host"].help(), "the remote host name");

  ASSERT_TRUE(map.parse("--host=http://github.com -p 25 -v -d", true));

  ASSERT_EQ(map["host"].get(), "http://github.com");
  ASSERT_EQ(map["port"].get(), "25");
  ASSERT_EQ(map["port"].get<int>(), 25);
  ASSERT_EQ(map["v"].get<bool>(), true);
  ASSERT_EQ(map["host"].get<std::string>(), "http://github.com");
}

TEST_F(tests_yacl, map_chaine_of_flags) {
  yacl::map map;

  map["h"].req<std::string>("h", "the remote host name");
  map["p"].opt<int>("p", "the remote host port", 80);
  map["v"].opt<bool>("v", "the remote host port",false);
  map["r"].opt<bool>("r", "enable report",true);
  map["Z"].opt<bool>("z", "deep debug",false);
  map["q"].opt<bool>("q", "quit if error",true);
  map["T"].opt<bool>("T", "trace",false);

  ASSERT_TRUE(map.parse("-qT -h http://github.com --port=25 -vrz", true));

  ASSERT_EQ(map["host"].get(), "http://github.com");
  ASSERT_EQ(map["port"].get<int>(), 25);
  ASSERT_EQ(map["v"].get<bool>(), true);
  ASSERT_EQ(map["r"].get<bool>(), false);
  ASSERT_EQ(map["z"].get<bool>(), true);
  ASSERT_EQ(map["q"].get<bool>(), false);
  ASSERT_EQ(map["T"].get<bool>(), true);
}

TEST_F(tests_yacl, map_with_custom_checks) {
  struct one_or_two {
    int operator()(const std::string &s) {
      return (s == "1" || s == "2");
    }

  };

  yacl::map map;

  map["x"].req<int>("x", "x should be one of {1,2}", yacl::oneof<int>(1,2));
  map["x"].req<int>("x", "x should be one of {1,2}", one_or_two());

  map["x"].req<std::string>("x", "x = {a,b}", yacl::oneof<std::string>("a", "b"));
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

  std::size_t i = 0;
  for (auto& it : yacl::convert(argc, argv) ) {
    ASSERT_EQ(it, std::string(argv[i++]));
  }
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
