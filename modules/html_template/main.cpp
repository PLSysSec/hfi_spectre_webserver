#include <iostream>
#include <string>
#include "Jinja2CppLight.h"
#include "../server_hostcalls.h"

using namespace Jinja2CppLight;

int main(int argc, char const *argv[])
{
    Template mytemplate( R"d(
        This is my {{avalue}} template.  It's {{secondvalue}}...
        Today's weather is {{weather}}.
        {% for j in range(32) %}b[{{j}}] = image[{{j}}];
        {% endfor %}{% endfor %}
        abc{% if its %}def{% endif %}ghi
        This is my {{avalue}} template.  It's {{secondvalue}}...
        Today's weather is {{weather}}.
        {% for j in range(32) %}b[{{j}}] = image[{{j}}];
        {% endfor %}{% endfor %}
        abc{% if its %}def{% endif %}ghi
    )d" );
    mytemplate.setValue( "avalue", 3 );
    mytemplate.setValue( "secondvalue", 12.123f );
    mytemplate.setValue( "weather", "rain" );
    mytemplate.setValue( "its", 200 );
    std::string result = mytemplate.render();
    server_module_string_result(result.c_str(), result.length());
    return 0;
}
