/**
 * @file
 * @author  Katarzyna Cwiklinska
 * @author  Krzysztof Kutt
 * @version 0.1
 */

#ifndef STATICGEST_H
#define STATICGEST_H

/**
 * Klasa odpowiedzialna za rozpoznawanie statycznych gestow.
 * Przeprowadza wstepna obrobke obrazu (rozdzielenie na kanaly, binaryzacja,
 * indeksacja), a nastepnie ustala ktory gest jest aktualnie pokazywany, oraz
 * w ktorym miejscu znajduje sie srodek dloni (do przesuwania kursora).
 *
 * Algorytm stworzono na bazie projektu PRMM103_HAND_GESTURES stworzonego przez
 * Michala Czajke, Grazyne Golakiewicz i Pawla Klimka.
 */

class StaticGest
{
    public:
        StaticGest();
    protected:
    private:
};

#endif // STATICGEST_H
