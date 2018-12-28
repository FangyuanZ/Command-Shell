#ifndef variable_h
#define variable_h
#include <iostream>
#include <map>
#include <string>

class variable
{
  //method1: set_var
  //method2: export_var
 public:
  std::map<std::string, std::string> var_map;
  void set_variable(std::string key, std::string value) {
    // var_map.insert(std::pair<char *, char *>(key, value));
    var_map[key] = value;
  }

  std::string get_variable(std::string key) { return var_map[key]; }
  bool find_key(std::string key) {
    std::map<std::string, std::string>::iterator it;
    for (it = var_map.begin(); it != var_map.end(); it++) {
      if (it->first == key) {
        return true;
      }
    }
    return false;
  }
  // void set_variable(){
  friend class myShell;
};
//std::map<char *, char *> variable::var_map;

#endif /* variable_h */
