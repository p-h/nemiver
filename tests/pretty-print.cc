#include <vector>
#include <string>

int
main()
{
    unsigned __attribute__((unused)) s;
    std::string s1 = "kélé", s2 = "fila", s3 = "saba";
    std::vector<std::string> v;
    v.push_back(s1);
    v.push_back(s2);
    v.push_back(s3);

    std::vector<std::string>::iterator i = v.begin ();
    i += 2;

    v.erase (i);

    s = v.size ();

    return 0;
}
