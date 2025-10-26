#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdexcept>

int main ( )
{
    std::srand ( static_cast< unsigned > ( std::time ( nullptr ) ) );
    int x = std::rand ( ) % 10;

    std::cout << "Random x = " << x << "\n";

    try
    {
        switch ( x % 4 )
        {
        case 0:
            std::cout << "Case 0: normal path\n";
            if ( x > 5 )
                throw std::runtime_error ( "x was too big in case 0" );
            break;
        case 1:
            std::cout << "Case 1: nested switch\n";
            switch ( x % 3 )
            {
            case 0:
                std::cout << "  Inner 0\n";
                break;
            case 1:
                std::cout << "  Inner 1\n";
                break;
            case 2:
                throw std::logic_error ( "Inner switch exception" );
            }
            break;
        case 2:
            std::cout << "Case 2: maybe throw\n";
            if ( x % 2 == 0 )
                throw std::runtime_error ( "Even number error" );
            else
                std::cout << "No exception here.\n";
            break;
        case 3:
            std::cout << "Case 3: fallthrough-like path\n";
            if ( x < 5 )
                std::cout << "x is small\n";
            else
                std::cout << "x is large\n";
            break;
        }
    }
    catch ( const std::logic_error &e )
    {
        std::cout << "Caught logic_error: " << e.what ( ) << "\n";
    }
    catch ( const std::runtime_error &e )
    {
        std::cout << "Caught runtime_error: " << e.what ( ) << "\n";
    }
    catch ( ... )
    {
        std::cout << "Caught unknown exception\n";
    }

    std::cout << "Program finished normally.\n";

    std::cin.get ( );

    return 0;
}
