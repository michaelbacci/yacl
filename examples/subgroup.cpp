#include <iostream>
#include "yacl.hpp"

using namespace std;

template<size_t size>
bool parse(yacl::map&map, char const * (&argv)[size]) {
    return map.parse(size, argv);
}
//template <class T> using magic_method = std::function<int(T*,int)>;

struct option_checks {
    int desc;

    int check_desc(int i) {
        return 0;
    }
};


struct check2 {
    int operator()(int i) {
        return 0;
    }
};

int main() {

    //compact declaration
    yacl::map map;

    map["d"].opt<int>("d", 0, "desc..", [](int i) { std::cout << "I like " << i ; return i;});
    map["d"].opt<int>("d", 0, "desc..", &option_checks::check_desc);

    map["shoot"]["x"].opt<bool>("x", false, "desc...");
    map["shoot"]["y"].opt<bool>("y", true, "desc...");
    map["shoot"]["z"].req<bool>("z", "desc...");
    map["shoot"][1].req<int>("pos1","desc...");
    map["shoot"][2].req<yacl::file>("pos2","desc...");

    map["host"].req<string>("h", "target host");
    map["port"].opt<int>("h", 80, "target host");

    const char * argv[] = {"program", "--host=80"};
    parse(map, argv);

//    map["shoot"][yacl::all].set<yacl::_strings>(yacl::required, "desc...");
//    map["shoot"]["val1"].set<int, yacl::range(1,100)>("f", yacl::required, "desc...");
//    map["shoot"]["val2"].set<int, yacl::range(1,100)>("f", yacl::required, "desc...");
//    map["shoot"]["func"].set<int, func>("f", yacl::required, "desc...");
//
//    map["atlas"]["k"].set<bool>("k", yacl::Option, "desc...");
//
//
//    map["operation"].set<std::string, yacl::oneof_map({"shoot", "atlas"})>("op", yacl::required, "desc...");
//    map["level"].set<int, yacl::oneof({1,3,5})>("", yacl::required, "desc...");
//
//    yacl::exclude(map["shoot"]["x"], map["shoot"]["y"]);
//    yacl::exclude(map["shoot"]["x"], map["shoot"]["y"]);
//    yacl::include(map["x"],map["y"]);
//    map["string:host:h:host name:required"];
//    map["int:port:p:network port:optional:range()"];
//
//    yacl::parse(map);
//    yacl::parse(map, yacl::using_posix);
//    yacl::parse(map, yacl::using_x).with(argc,argv);
//
//    map["host"]["submodule"] = {};
//    map["host"] << yacl::desc << "Here I put my long..long description";
//    map["host"] << yacl::check <<
//    map["host"] >> yacl::call >> yac::run([&](){ std::cout << "Host is " << map["host"]; });

/*
 *
 * --op=shoot -x -y
 *
 *
 *  --operation=shoot -x -y f.txt g.jpg A1.xml B1.vtk A2.xml B2.vtk
 *  --
 *
 * */


    //il piu semplice:

//    for (auto arg : yacl::parse(argc,argv)) {
//        std::cout << arg << " at pos " << arg.position();
//    }
//
//    yacl::map input;
//    input = yacl::parse(argc,argv);
//
//    for (int i = 0; i < input.size(); ++i) {
//        std::cout << input[i].as<string>() << " at pos " << i;
//    }
//
//

    std::cout << map["host"];

    return 0;
}