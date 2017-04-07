#pragma once

#include <string>
#include <ostream>
#include <vector>
#include <functional>
#include <map>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <sstream>

namespace yacl {
class map;
class info;
typedef struct {} all;
typedef struct {} file;


template <class T>
struct read {
  T operator()(T data) { return data; }
};

template <class T>
struct range {
  T begin;
  T end;
  range(const T& b, const T& e) : begin(b), end(e) {}
  T operator()(T data) { return (begin <= data && data <= end); }
};

template <class T>
struct file_exist {
  T operator()(T data) { return true; }
};

class option_abstract {
 public:
  enum option_type {
    OPTIONAL,
    REQUIRED
  };

  ~option_abstract() {}

  virtual void set_help(const std::string& s) = 0;
  virtual std::string& get_help() = 0;

  virtual void set_long_name(const std::string& s) = 0;
  virtual std::string& get_long_name() = 0;

  virtual void set_short_name(const std::string& s) = 0;
  virtual std::string& get_short_name() = 0;

  virtual void set_type(const option_type s) = 0;
  virtual option_type get_type() = 0;
};

class option_methods : public option_abstract {
 protected:
  std::string help;
  std::string long_name;
  std::string short_name;
  option_type type;

 public:

  virtual void set_help(const std::string& s) { help = s; }
  virtual std::string& get_help() { return this->help; }

  virtual void set_long_name(const std::string& s) { long_name = s; }
  virtual std::string& get_long_name() { return long_name; }

  virtual void set_short_name(const std::string& s) { short_name = s; }
  virtual std::string& get_short_name()  { return short_name; }

  virtual void set_type(const option_type s) { type = s; }
  virtual option_type get_type() { return type; }
};

template <class T>
class filter_abstract {
 public:
  virtual T filter(const std::string &s) = 0;
  ~filter_abstract() {}
};


template <class T>
class filter_string_stream : public filter_abstract<T> {
 public:
  virtual T filter(const std::string &s) {
    T t;
    std::stringstream ss;

    try {
      ss << s;
      //ss >> t;
    } catch(...) {
      throw std::runtime_error("String [" + s + "] can't be converted to the required type");
    }

    return t;
  }

};

template <class T>
class option: public option_methods, public filter_string_stream<T> {
 protected:
  std::string data;
 public:

  option() {}
  option(const std::string &data) : data(data){}

  virtual const std::string& get_data() const { return data; }
};

template <class T, class F>
class option_with_object_filter: public option<T> {

 public:
  option_with_object_filter(F f) : f(f) {}

  virtual T filter(const std::string &s) {
    return f(s);
  }

 private:
  F f;
};

template <class T>
class option_with_object_filter<T, std::function<T(T)>>: public option<T> {

 public:
  option_with_object_filter(std::function<T(T)> f) : f(f) {}

  virtual T filter(T v) {
    return f(v);
  }

 private:
  std::function<T(T)> f;
};

template <class T>
class option_with_lambda_filter: public option<T> {
 public:
  option_with_lambda_filter(std::function<T(T)> f) : f(f) {}

  virtual T filter(const std::string &s) {
    return f(option<T>::filter(s));
  }

 private:
  std::function<T(T)> f;
};

template <class T>
class filter_oneof : public filter_string_stream<T> {
 public:
  filter_oneof(const std::vector<T>& candidates) : candidates(candidates) {}

  T operator()(const std::string &value) {
    T filter_value = filter_string_stream<T>::filter(value);

    if (std::find(candidates.begin(), candidates.end(), filter_value) == candidates.end())
      throw std::domain_error("The input value [" + value + "] is not allowed");

    return filter_value;
  }

 private:
  std::vector<T> candidates;
};

template <class T, typename... Args>
filter_oneof<T> oneof(T first, Args... args) {
  return filter_oneof<T>(std::vector<T>({first, args...}));
}

class map {

 private:
  std::string description;

  typedef std::shared_ptr<map> ptr_map;
  typedef std::shared_ptr<option_abstract> ptr_option;

  ptr_option single_option;
  std::map<unsigned int, ptr_option> int_options;
  std::unordered_map<std::string, ptr_map> multi_options;

  void add(ptr_option op,
           const std::string& long_name,
           const std::string short_name,
           const std::string help,
           const option_abstract::option_type type) {
    op->set_long_name(description);
    op->set_short_name(short_name);
    op->set_help(help);
    op->set_type(type);
    single_option = op;
  }

//  template <class T, class P>
//  void add(std::string long_name, std::string short_name, const T data=T(), P f=P()) {
//  }

  void check_condition() const throw(std::domain_error) {
    if (description.empty())
      throw std::domain_error("description parameter missing");
  }

 public:

  map()  {}
  map(const std::string&s) :
      description (s),
      single_option(new option_methods())
  {}

  template <class T>
  void req(std::string short_name, std::string help) {
    check_condition();
    add(std::make_shared<option<T>>(),
        description,
        short_name,
        help,
        option_abstract::REQUIRED);
  }

  template <class T>
  void req(std::string short_name, std::string help, std::function<T(T)> filter) {
    check_condition();
    add(std::make_shared<option_with_lambda_filter<T>>(filter),
        description,
        short_name,
        help,
        option_abstract::REQUIRED);
  }

  template <class T, class F>
//  typename std::enable_if<std::is_convertible<F, std::function<T(T)>>::value>::type
//  typename std::enable_if<std::is_convertible<decltype(&F::operator()), std::function<T(T)>>::value>::type
  void req(std::string short_name, std::string help, F filter=F()) {
    check_condition();
    add(std::make_shared<option_with_object_filter<T, F>>(filter),
        description,
        short_name,
        help,
        option_abstract::REQUIRED);
  }

  template <class T>
  void opt(std::string short_name, std::string help, T val) {
    check_condition();
  }

  template <class T>
  void opt(std::string short_name, std::string help, T val, std::function<T(T)> f) {
    check_condition();
  }

  template <class T, class P>
  void opt(std::string short_name, std::string help, T val, P f=P()) {
    check_condition();
  }

  template <class T>
  T get() {
    check_condition();
    option<T> *data = dynamic_cast<option<T>*>(single_option.get());

    return data->filter(data->get_data());
  }

  std::string get() {
    check_condition();
    return "";
  }

  int size() const {
    return multi_options.size();
//    +
//        std::accumulate(multi_options.begin(), multi_options.end(),0,
//                        [](const size_t p, const std::pair<std::string,size_t>& map) {
//                          return p+map.second;
//                        });
  }

  std::string& help() const {
    check_condition();
    option_methods *data = dynamic_cast<option_methods*>(single_option.get());
    return single_option->get_help();
  }

  map& operator[](const std::string& s) {
    if (multi_options.find(s) != multi_options.end()) {
      return *multi_options[s];
    }

    multi_options[s] = std::make_shared<map>(s);
    return *multi_options[s];
  }

  map& operator[](unsigned int pos) {
    if (int_options.find(pos) != int_options.end()) {

    }

    return *this;
  }

  map& operator[](all l) {
    return *this;
  }

  map& operator=(std::vector<std::string>) {
    return *this;
  }

  bool del() const {
    return true;
  }

  std::string to_string() {
    return "";
  }

  bool parse(const int argc, const char *argv[]) {
    return true;
  }

  /**
   * parsing ignoring the argv[0] = program_name
   * ex: parse("-a --host=github.com")
   */
  bool parse(std::string v) {

  }
};

// Utils

std::ostream& operator<<(std::ostream&os, map&m) {
  os << m.to_string();
  return os;
}



struct convert {

  typedef char** type_argv;

  convert(int argc, type_argv argv)
      : argc_(argc)
      , argv_(argv)
  {
    if (argc < 1)
      return;

    s_argv_ = std::string(argv[0]);
    for (auto&s : std::vector<std::string>(argv+1, argv + argc))
      s_argv_ += " " + s;
  }

  convert(std::string s_argv)
      : s_argv_(s_argv)
  {
    std::stringstream ss;
    std::string s_tmp;
    std::vector<std::string> v_tmp;

    ss << s_argv_;
    while (ss >> s_tmp) {
      v_tmp.push_back(s_tmp);
    }

    argc_ = v_tmp.size();
    if (argc_ == 0) {
      argv_ = nullptr;
      return;
    }

    argv_ = new char*[argc_];
    char** it = argv_;

    for (auto&s : v_tmp) {
      char *i = new char[s.length() + 1];
      std::copy(s.begin(), s.end(), i);
      i[s.length()] = '\0';
      *it++ = i;
    }
  }

  convert& operator>>(std::string& s) {
    s = std::string(s_argv_);
    return *this;
  }

  convert& operator>>(int& argc) {
    argc = argc_;
    return *this;
  }

  convert& operator>>(type_argv& argv) {
    argv = argv_;
    return *this;
  }

 private:
  int argc_;
  type_argv argv_;
  std::string s_argv_;
};
void to_argv(std::string v, int* argc, char*** argv) {
  std::stringstream ss;
  std::string tmp;
  ss << v;

  while (ss >> tmp) {

  }
}


}
