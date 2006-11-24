#include <iostream>

template <class Tx, class Ty>
class plot
{
    public:
    typedef enum
    {
      LINE,
      POINT,
      BAR
    } plot_type ;

    private:
    plot_type m_type ;

    public:
    plot () :
        m_type (LINE)
    {}

    plot_type type () const {return m_type;}
    void type (plot_type a_type) {m_type = a_type;}
};//plot

std::ostream&
operator<< (std::ostream &a_out, const plot<int, int> &a_plot)
{
    switch (a_plot.type ()) {
        case plot<int, int>::LINE :
            a_out << "plot::LINE" ;
            break;
        case plot<int, int>::POINT :
            a_out << "plot::POINT" ;
            break ;
        case plot<int, int>::BAR :
            a_out << "plot::BAR" ;
            break ;
    }
    return a_out ;
}

int
main ()
{
    plot<int, int> p ;
    p.type (plot<int, int>::BAR) ;
    std::cout << p << std::endl ;
    return 0 ;
}

