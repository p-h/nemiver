#include <iostream>
#include <map>
#include <vector>

using std::vector;
using std::map;

template <class Tx, class Ty>
class plot
{
    public:
    typedef enum
    {
      LINE,
      POINT,
      BAR
    } plot_type;
    typedef map<Tx, Ty> dataset_t;

    private:
    plot_type m_type;
    vector<dataset_t> m_datasets;

    public:
    plot () :
        m_type (LINE)
    {}

    plot_type type () const {return m_type;}
    void type (plot_type a_type) {m_type = a_type;}
    void add_dataset(dataset_t& set) { m_datasets.push_back(set); }
};//plot

std::ostream&
operator<< (std::ostream &a_out, const plot<int, int> &a_plot)
{
    switch (a_plot.type ()) {
        case plot<int, int>::LINE :
            a_out << "plot::LINE";
            break;
        case plot<int, int>::POINT :
            a_out << "plot::POINT";
            break;
        case plot<int, int>::BAR :
            a_out << "plot::BAR";
            break;
    }
    return a_out;
}

int
main ()
{
    plot<int, int> p;
    p.type (plot<int, int>::BAR);
    std::cout << p << std::endl;
    map<int, int> set1;
    set1[0] = 0;
    set1[1] = 1;
    set1[2] = 2;
    set1[3] = 3;
    set1[4] = 4;
    set1[5] = 5;
    p.add_dataset(set1);
    return 0;
}

