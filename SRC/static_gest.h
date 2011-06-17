/**
 * @file
 * @author  Katarzyna Cwiklinska
 * @author  Krzysztof Kutt
 * @version 0.1
 */

#ifndef STATIC_GEST_H
#define STATIC_GEST_H

/**
 * Klasa odpowiedzialna za rozpoznawanie statycznych gestow.
 * Przeprowadza wstepna obrobke obrazu (rozdzielenie na kanaly, binaryzacja,
 * indeksacja), a nastepnie ustala ktory gest jest aktualnie pokazywany, oraz
 * w ktorym miejscu znajduje sie srodek dloni (do przesuwania kursora).
 *
 * Algorytm stworzono na bazie projektu PRMM103_HAND_GESTURES stworzonego przez
 * Michala Czajke, Grazyne Golakiewicz i Pawla Klimka.
 */

class static_gest
{
    public:
        static_gest();
    protected:
    private:
};

#endif // STATIC_GEST_H
