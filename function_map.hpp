#ifndef FUNCTION_MAP_INCLUDED
#define FUNCTION_MAP_INCLUDED
#include<map>
typedef std::map<const std::string, std::pair<std::string, void*>> function_map_t;


template<class T>
class Singleton
{
private:
   Singleton();

public:
   static T& get()
   {
      static T INSTANCE;
      return INSTANCE;
   }
};

typedef Singleton<function_map_t> function_map;

class Register
{
public:
	Register(std::string name, std::string kind, void*function) {
		function_map::get()[name] = std::pair<std::string, void*>(kind, function);
	}
};

#define FUNCTION(kind, f) Register f##_reg(#f, kind, (void*)f)

#endif
