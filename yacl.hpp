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
#include <deque>
#include <cctype>
#include <clocale>
#include <type_traits>

namespace yacl {


namespace tdata {

template<typename T>
class TData;

class BData {
 public:

  bool const is_setted() const noexcept {
    return bdata_ != nullptr;
  }

  const std::type_info &type_info() const throw(std::bad_typeid) {
    if (!is_setted())
      throw std::bad_typeid();

    return *info_;
  }

  template<typename T>
  void set_data(const T &t) noexcept {
    info_ = (std::type_info *) &typeid(T);
    auto d = std::make_shared<TData<T>>();
    d->set_data(t);
    bdata_ = d;
  }

  template<typename T>
  void set_data(T &&t) noexcept {
    info_ = (std::type_info *) &typeid(T);
    auto d = std::make_shared<TData<T>>();
    d->set_data(t);
    bdata_ = d;
  }

  template<typename T>
  const T &get_data() const throw(std::logic_error, std::bad_cast) {
    if (!is_setted()) {
      throw std::logic_error("No data are available");
    }

    if ((std::type_info *) &typeid(T) != info_)
      throw std::bad_cast();

    return ((TData<T> *) (bdata_.get()))->get_data();
  }

  template<typename T>
  const T &get_data_safe() noexcept {
    if (!is_setted() || (std::type_info *) &typeid(T) != info_) {
      T t;
      auto d = std::make_shared<TData<T>>();
      d->set_data(t);
      bdata_ = d;
    }

    return ((TData<T> *) (bdata_.get()))->get_data();
  }

 protected:
  std::shared_ptr<BData> bdata_;
  std::type_info *info_;
};

template<typename T>
class TData : public BData {
 public:

  void set_data(const T &t) noexcept { data_ = t; }
  void set_data(T &&t) noexcept { data_ = std::move(t); }
  const T &get_data() const noexcept { return data_; }

 protected:
  T data_;
};
}


class map;
class info;
typedef struct {} all;
typedef struct {} file;




class convert {
  typedef char** type_argv;
  int argc_;
  type_argv argv_;
  std::string s_argv_;
  std::vector<std::string> v_argv_;

 public:

  convert(int argc, type_argv argv)
      : argc_(argc)
      , argv_(argv)
      , v_argv_(argv, argv + argc)
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

    ss << s_argv_;
    while (ss >> s_tmp) {
      v_argv_.push_back(s_tmp);
    }

    argc_ = v_argv_.size();
    if (argc_ == 0) {
      argv_ = nullptr;
      return;
    }

    argv_ = new char*[argc_];
    char** it = argv_;

    for (auto&s : v_argv_) {
      char *i = new char[s.length() + 1];
      std::copy(s.begin(), s.end(), i);
      i[s.length()] = '\0';
      *it++ = i;
    }
  }

  auto begin() -> decltype(v_argv_.begin()) { return v_argv_.begin(); }
  auto end() -> decltype(v_argv_.end()) { return v_argv_.end(); }

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

};

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

namespace filters {

struct filter {
  ~filter() {}
  virtual std::string operator()(const std::string &v) = 0;
};

struct lower_case : public filter {
  virtual std::string operator()(const std::string &v) {
    std::string lower_str(v);
    //std::transform(v.cbegin(), v.cend(), lower_str.begin(), std::tolower);
    return lower_str;
  }
};

}


class OptionProgrammable {
 public:
  typedef std::function<void(const std::string &val)> reader;

    template <typename T>
    OptionProgrammable add_reader(T f) {
      readers_.push_back(f);
      return *this;
    }

    template <typename T, typename O, typename F>
    OptionProgrammable& assign_to(O o, F f) {
      assign_ = [o, f](const std::string &val) {
        std::stringstream ss;
        T data;
        ss << val;
        ss >> data;
        if (o == nullptr)
          throw std::runtime_error("assing_to() methond has null object");
        (o->*f)(data);
      };

      return *this;
    }

//    template <typename F>
//    XmlProgrammable& filter_with(F f) {
//      filter_ = std::make_shared<F>();
//      return *this;
//    }

 protected:
  std::shared_ptr<filters::filter> filter_;
  reader assign_;
  std::vector<reader> readers_;
};

class OptionAbstract {
 public:
  enum option_type {
    OPTIONAL,
    REQUIRED
  };

  ~OptionAbstract() {}

  virtual void set_help(const std::string& s) = 0;
  virtual std::string& get_help() = 0;

  virtual void set_long_name(const std::string& s) = 0;
  virtual std::string& get_long_name() = 0;

  virtual void set_short_name(const std::string& s) = 0;
  virtual std::string& get_short_name() = 0;

  virtual void set_type(const option_type s) = 0;
  virtual option_type get_type() = 0;
};

class OptionMethod : public OptionAbstract {
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
class FilterAbstract {
 public:
  virtual T filter(const std::string &s) = 0;
  ~FilterAbstract() {}
};


template <class T>
class FilterStringStream : public FilterAbstract<T> {
 public:
  virtual T filter(const std::string &s) {
    T t;
    std::stringstream ss;
    if (!(ss<<s && ss>>t && ss.eof()))
      throw std::bad_cast();

    return t;
  }

};


template <class T>
class Option: public OptionMethod, public FilterStringStream<T> {
 protected:
  std::string data;
  T default_value;
  T cmdline_value;
 public:

  Option() {}
  //Option(const std::string &data) : data(data){}

  void set_default_value(const T& v) { default_value = v;}
  T& get_default_value() { return default_value; }

  void set_cmdline_value(const T& v) { cmdline_value = v;}
  T& get_cmdline_value() { return cmdline_value; }

  virtual const std::string& get_data() const { return data; }
};

template <class T, class F>
class option_with_object_filter: public Option<T> {

 public:
  option_with_object_filter(F f) : f(f) {}

  virtual T filter(const std::string &s) {
    return f(s);
  }

 private:
  F f;
};

template <class T>
class option_with_object_filter<T, std::function<T(T)>>: public Option<T> {

 public:
  option_with_object_filter(std::function<T(T)> f) : f(f) {}

  virtual T filter(T v) {
    return f(v);
  }

 private:
  std::function<T(T)> f;
};

template <class T>
class option_with_lambda_filter: public Option<T> {
 public:
  option_with_lambda_filter(std::function<T(T)> f) : f(f) {}

  virtual T filter(const std::string &s) {
    return f(Option<T>::filter(s));
  }

 private:
  std::function<T(T)> f;
};

template <class T>
class filter_oneof : public FilterStringStream<T> {
 public:
  filter_oneof(const std::vector<T>& candidates) : candidates(candidates) {}

  T operator()(const std::string& value) {
    T filter_value = FilterStringStream<T>::filter(value);

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
  std::deque<std::string> warnings;
  std::string value;
  bool enabled;

  typedef std::shared_ptr<map> ptr_map;
  typedef std::shared_ptr<OptionAbstract> ptr_option;

  ptr_option single_option;
  std::unordered_map<unsigned int, ptr_option> int_options;
  std::unordered_map<std::string, ptr_map> multi_options;

  void add(ptr_option op,
           const std::string& long_name,
           const std::string short_name,
           const std::string help,
           const OptionAbstract::option_type type) {
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
      enabled(false),
      single_option(new OptionMethod())
  {}

  template <class T>
  void req(std::string short_name, std::string help) {
    check_condition();
    add(std::make_shared<Option<T>>(),
        description,
        short_name,
        help,
        OptionAbstract::REQUIRED);
  }

  template <class T>
  void req(std::string short_name, std::string help, std::function<T(T)> filter) {
    check_condition();
    add(std::make_shared<option_with_lambda_filter<T>>(filter),
        description,
        short_name,
        help,
        OptionAbstract::REQUIRED);
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
        OptionAbstract::REQUIRED);
  }

  template <class T>
  void opt(std::string short_name, std::string help, T val) {
    check_condition();
    auto opt = std::make_shared<Option<T>>();
    opt->set_default_value(val);

    add(std::move(opt),
        description,
        short_name,
        help,
        OptionAbstract::OPTIONAL);
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
  T as() const {
    check_condition();

    //add check had parsed
    Option<T> *data = dynamic_cast<Option<T>*>(single_option.get());
    if (!data)
      throw std::domain_error("Conversion not allowed");

    if (enabled) return data->get_cmdline_value();

    if (data->get_type() == OptionAbstract::REQUIRED)
      throw std::runtime_error("Required missing value");

//    return data->filter(data->get_default_value());
/*
    if (std::is_same<T, bool>::value)
      return data->get_cmdline_value();

    if (enabled) return data->filter(value);

    if (data->get_type() == OptionAbstract::REQUIRED)
      throw std::runtime_error("Required missing value");

    return data->get_default_value();
*/
  }

  std::string as_string() {
    check_condition();
    return value;
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
    OptionMethod *data = dynamic_cast<OptionMethod*>(single_option.get());
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

  bool has_space(const std::string& v) {
    return (std::count(v.begin(), v.end(), ' ') > 0);
  }

  bool has_long_option(const std::string& v, std::string& opt_name, std::string& opt_val) {
    if (!(v.length() > 1)) return false;
    if (!(v[0] == '-' && v[1] == '-')) return false;
    if (!(v.length() >= 5)) throw std::domain_error("Invalid argument [" + v + "]");
    if (!((bool) std::isalpha(v[2]))) throw std::domain_error("Invalid argument [" + v + "]");
    if (!(std::count(v.begin(), v.end(), '=') == 1)) return false;

    auto eq_pos = std::find(v.begin(), v.end(), '=');
    if (!(eq_pos != v.end()-1)) throw std::domain_error("Incomplete argument [" +  v + "]");

    opt_name = std::string(v.begin()+2, eq_pos);
    opt_val = std::string(eq_pos+1, v.end());

    return true;
  }

  bool has_single_short_option(const std::string& v, std::string& opt_name) {
    if (!(v.length() == 2)) return false;
    if (!(v[0] == '-')) return false;
    if (!((bool) std::isalpha(v[1]))) throw std::domain_error("Invalid argument [" + v + "]");

    opt_name = std::string(v.begin()+1, v.end());
    return true;
  }

  bool has_positional_option(const std::string& v, std::string& opt_val) {
    if (!(v.length())) return false;
    if (!(v[0] != '-')) throw std::domain_error("The argument [" + v + "] should be a positional value");

    opt_val = v;
    return true;
  }


  /**
   * parsing ignoring the argv[0] = program_name
   * ex: parse("-a --host=github.com")
   */

  bool parse(convert c, bool missing_program_name = false, bool ignore_program_name=true) {
    auto it = c.begin();

    if (!missing_program_name && ignore_program_name)
      ++it;

      for (; it != c.end(); ++it) {
        if (has_space(*it))
          throw std::domain_error("ERROR : found space character in arguments");

        std::cout << "ANA [" << *it << "]" << std::endl;
          std::string opt_name;
          std::string opt_val;
        //CASE '--Option=<str>':
        {
          if (has_long_option(*it, opt_name, opt_val)) {

            std::cout << "MATCH " << opt_name << " = " << opt_val << std::endl;

            if (multi_options.find(opt_name) == multi_options.end()) {
              warnings.push_back("Option [" + *it + "] is ignored");
              continue;
            }

            multi_options[opt_name]->value = opt_val;
//TODO:auto parsing and assign value!!!
            //multi_options[opt_name]->single_option->se = opt_val;
            multi_options[opt_name]->enabled = true;

            continue;
          }
        }

        //CASE '-s' single short Option :
        {
          if (has_single_short_option(*it, opt_name)) {

            std::cout << "MATCH SHORT " << opt_name << " = " << opt_val << std::endl;

            bool matched = false;
            for (auto& ik : multi_options) {
              if (ik.second->single_option->get_short_name() == opt_name) {
                opt_name = ik.second->description;
                matched = true;
                std::cout << "short matched ";
                if (dynamic_cast<FilterAbstract<bool>*>(ik.second->single_option.get())) {
                  ik.second->enabled = true;
                  std::cout << " --is bool";
                } else {
                  std::cout << " --is value ";
                  ++it;
                  if (it == c.end()) throw std::domain_error("The Option [" + *it + "] require a value");
                  if (!has_positional_option(*it, opt_val)) {
                    throw std::domain_error("The Option [" + *it + "] require a value");
                  }
                  break;
                }
              }
            }

            if (!matched) {
              warnings.push_back("Option [" + *it + "] is ignored");
              continue;
            }

            multi_options[opt_name]->value = opt_val;
            multi_options[opt_name]->enabled = true;
            continue;
          }
        }

      }

    std::cout << "\nWARNING";
    for (auto&it : warnings)
      std::cout << std::endl << it;
    return true;
  }

  bool parse(std::string v, bool missing_program_name = false, bool ignore_program_name=true) {
    return parse(convert(v), missing_program_name, ignore_program_name);
  }

  bool parse(int argc, char **argv, bool missing_program_name = false, bool ignore_program_name=true) {
    return parse(convert(argc,argv), missing_program_name, ignore_program_name);
  }

};

// Utils

std::ostream& operator<<(std::ostream&os, map&m) {
  os << m.to_string();
  return os;
}

}
