/**
 * @file
 * @author  Katarzyna Cwiklinska
 * @author  Krzysztof Kutt
 * @version 0.1
 */

#ifndef SYSTEMAPI_H
#define SYSTEMAPI_H

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
        SystemAPI();
    protected:
    private:
};

#endif // SYSTEMAPI_H
