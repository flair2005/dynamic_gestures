/**
 * @file
 * @author  Katarzyna Cwiklinska
 * @author  Krzysztof Kutt
 * @version 0.1
 */

#ifndef SYSTEMAPI_H
#define SYSTEMAPI_H

//definicja potrzebna, aby korzystac z WinAPI (biblioteki WinAPI sprawdzają
//ten parametr i jeżeli nie jest dobrze ustawiony to nie zalaczaja sie)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>

/**
 * Klasa odpowiedzialna za komunikację z systemem operacyjnym Windows. Nic nie
 * stoi na przeszkodzie, aby w przyszłości napisać odpowiednik tej klasy pod
 * inne systemy operacyjne - należy tylko utrzymać jednolite nazewnictwo funkcji.
 *
 * Klasa będzie odpowiadać za sterowanie kursorem, emulację kliknięć myszy,
 * minimalizację/maksymalizację okien.
 */

class SystemAPI
{
    public:
        static void pressLeft();        //naciska lewy klawisz
        static void releaseLeft();      //puszcza lewy klawisz
        static void pressRight();       //naciska prawy klawisz
        static void releaseRight();     //puszcza prawy klawisz
        static void move(int x, int y); //przesuwa kursor o x w poziomie i y w pionie
    protected:
    private:
};

#endif // SYSTEMAPI_H
