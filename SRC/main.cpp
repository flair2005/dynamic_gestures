/**
 * @file
 * @author  Katarzyna Cwiklinska
 * @author  Krzysztof Kutt
 * @version 1.0
 *
 * @section LICENSE
 *
 * Copyright (c) 2011, Katarzyna Cwiklinska & Krzysztof Kutt
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * Celem projektu jest rozpoznawanie dynamicznych gestow wykonywanych przy
 * pomocy dloni. Wybrane gesty sluza do emulacji komunikatow sterujących
 * interfejsem graficznym komputera (otwieranie okien, zamykanie okien,
 * przesuwanie kursora.)  Do rozpoznawania uzyto algorytmu HMM (ang.
 * Hidden Markov Model).
 *
 */

//definicja potrzebna, aby korzystac z WinAPI (biblioteki WinAPI sprawdzają
//ten parametr i jeżeli nie jest dobrze ustawiony to nie zalaczaja sie)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <windows.h>

//te dwa byly zalaczone w projekcie Hand Gestures
//jezeli nam tez beda potrzebne to sie odkomentuje
#include <math.h>
#include <algorithm>

using namespace std;

/* int main()
{
    cout << "Hello world!" << endl;
    return 0;
} */

/******************* KASUJ *********************/

#define LUT_SIZE 255 //wielkosc tablicy sklejen
// zdefiniowane gesty
#define CALA_DLON 4
#define PALEC 1
#define PIESC 0
#define INNY -1

IplImage* img;	//przechowuje przechwycony obraz
IplImage* Y;	//przechowuje kanał Y
IplImage* U;	//przechowuje kanał U
IplImage* V;	//przechowuje kanał V
IplImage* Ub;	//obraz po binaryzacji U
IplImage* Vb;	//obraz po binaryzacji V
IplImage* B;	//Ub&Vb
IplImage* T;	//obraz przestrzeni między palcami
IplImage* tmp;	//używany do tymczasowych obliczeń
int width,height,screenw,screenh;	//szerokość i wysokość przechwyconego obrazu i ekranu monitora
int progU,progV,progP;	//progi binaryzacji i próg stosunku powierzchni dłoni do powierzchni znalezionych dziur
int x,y,xt,yt,xs,xf,ys,yf;	// położenia kursora, środka dłoni oraz skrajnych punktów dłoni
int ruch;	//zmienna decyzyjna o przechwyceniu kursora
int **tab;	//przechowuje obraz do indeksacji
int LUT[LUT_SIZE][2];	//tablica sklejeń indeksacji i tablica pol znalezionych przestrzeni miedzy palcami
int ileDziur;	//ilosc przestrzeni miedzy palcami o odpowiednich proporcjach do dłoni
int gestWykryty;
int gestUstalony;
int gestTmp;
int gestLicznik; //licznik dla ustalaniea gestow

int kamera;
char * sciezka;

void lewyNacisnij(){ //naciska lewy klawisz
	INPUT input;
	ZeroMemory(&input,sizeof(INPUT));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1,&input,sizeof(INPUT));
}

void lewyPusc(){ //puszcza lewy klawisz
	INPUT input;
	ZeroMemory(&input,sizeof(INPUT));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1,&input,sizeof(INPUT));
}

void prawyNacisnij(){ //naciska prawy klawisz
	INPUT input;
	ZeroMemory(&input,sizeof(INPUT));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	SendInput(1,&input,sizeof(INPUT));
}

void prawyPusc(){ //puszcza prawy klawisz
	INPUT input;
	ZeroMemory(&input,sizeof(INPUT));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	SendInput(1,&input,sizeof(INPUT));
}

void przesun(int x, int y){ //przesuwa kursor o x w poziomie i y w pionie
	POINT pos;
	ZeroMemory(&pos,sizeof(POINT));
	GetCursorPos(&pos);
	SetCursorPos(pos.x+x,pos.y+y);
}
/*
	SERCE PROGRAMU:
	-	binaryzacja
	-	wykrywanie gestu
	-	obsluga kursora
*/
void binary(){
	Ub=cvCloneImage(U);
	Vb=cvCloneImage(V);
	int count=0; // ilość punktów należących do dłoni
	//ustawienie zmiennych do wykrycia położenia kursora
	xs=width;
	xf=0;
	ys=height;
	yf=0;
	// binaryzacja filtrem kombinowanym na kanałach U i V
	// wynikbinaryzacji B = U&V
	// progi binaryzacji progU i progV mozna dynamicznie zmieniac podczas dzialania programu
	for(int i = 0; i<height; i++){
		for(int j=0; j<width; j++){
			uchar *u = &CV_IMAGE_ELEM(Ub,uchar,i,j);
			uchar *v = &CV_IMAGE_ELEM(Vb,uchar,i,j);
			uchar *b = &CV_IMAGE_ELEM(B,uchar,i,j);
			if(u[0]>=progU) u[0]=255;
			else u[0]=0;
			if(v[0]>=progV) v[0]=255;
			else v[0]=0;
			if(u[0]==255 && v[0]==255) b[0]=255;
			else b[0]=0;
			if(i==0 || i==height-1 || j==0 || j==width-1){
				b[0]=0;
			}
		}
	}

	// zamknięcie i otwarcie dla poprawy kształtu dłoni
	cvMorphologyEx(B,B,tmp,cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_ELLIPSE),CV_MOP_CLOSE,1);
	cvMorphologyEx(B,B,tmp,cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_ELLIPSE),CV_MOP_OPEN,1);


	//wycznaczenie skrajnych punktow dłoni
	//policzenie powierzchni dłoni
	for(int i = 0; i<height; i++){
		for(int j=0; j<width; j++){
			uchar *b = &CV_IMAGE_ELEM(B,uchar,i,j);
			if(b[0]==255){
				count++; // tu liczymy powierzchnie
				//kolejne linijki ustalaja położenie skrajnych punktów dłoni
				if(j<xs)xs=j;
				if(j>xf)xf=j;
				if(i<ys)ys=i;
				if(i>yf)yf=i;
			}
		}
	}

	if(count==0){
		count++;
	}
	else{//wyznaczenie środka dłoni jeśli został znaleziony jakiś punkt.
		if(xs!=xf){
			x=(int)screenw-(xs+(xf-xs)/2)*screenw/width;
			xt=(int)width-(xs+(xf-xs)/2);
		}
		if(ys!=yf){
			y=(int)(ys+(yf-ys)/2)*screenh/height;
			yt=(int)(ys+(yf-ys)/2);
		}
	}
	//wyznazcamy figure wypukłą otaczającą dłoń
	int i,licznik,hullcount;

	CvPoint pt0;

	CvPoint* points = (CvPoint*)malloc( count * sizeof(points[0]));
	int* hull = (int*)malloc( count * sizeof(hull[0]));
	CvMat pointMat = cvMat( 1, count, CV_32SC2, points );
    CvMat hullMat = cvMat( 1, count, CV_32SC1, hull );
	//zbieranie informacji na temat punktow nalezacych do dłoni
	licznik=0;
	for(int i = 0; i<height; i++)
		for(int j=0; j<width; j++){
			uchar *b = &CV_IMAGE_ELEM(B,uchar,i,j);
			if(b[0]==255){
				pt0.x=j;
				pt0.y=i;
				points[licznik]=pt0;
				licznik++;
			}
		}
	// cvConvexHull2 wyznacza nam punkty będące wierzchołkami wypukłej fugury
	cvConvexHull2( &pointMat, &hullMat, CV_CLOCKWISE, 0 );
	hullcount = hullMat.cols;

	cvZero(tmp);

	CvPoint* points1 = (CvPoint*)malloc( hullcount * sizeof(points[0]));

	for( i = 0; i < hullcount; i++ ){
		points1[i] = points[hull[i]];
	}

	//rysujemy figure wypukla
	cvFillPoly(tmp,&points1,&hullcount,1,cvScalar(255),8,0);
	//roznica wypuklej figury i dloni daje nam przestrzenie miedzy palcami
	cvSub(tmp,B,T,0);

	free(points);
	free(points1);
	free(hull);

	//dla ulatwienia sobie rzycia obraz przechowujacy przestrzenie miedzy palcami przekształcamy do tablicy int
	for(int i = 0; i<height; i++){
		for(int j=0; j<width; j++){
			uchar *t = &CV_IMAGE_ELEM(T,uchar,i,j);
			if(t[0]==255){
				tab[j][i]=255;
			}
			else{
				tab[j][i]=0;
			}
		}
	}

	// przystępujemy do indeksacji z tablicą sklejeń
	// algorytm zaczerpnięty z książki prof. Tadeusiewicza

	int a,b,c,d,X,l,mini,maks;
	l=0;


	//indeksacja 1 tura
	for(int i=1;i<width-1;i++){
		for(int j=1;j<height-1;j++){
			a=tab[i-1][j-1];
			b=tab[i][j-1];
			c=tab[i+1][j-1];
			d=tab[i-1][j];
			X=tab[i][j];
			maks=max(max(a,b),max(c,d));
			if(a!=0 && b!=0 && c!=0 && d!=0) mini=min(min(a,b),min(c,d));
			if(a>0){ //a>0
				mini=a;
				if(b>0){
					mini=(mini<b)?mini:b;
					if(c>0){
						mini=(mini<c)?mini:c;
						if(d>0) mini=(mini<d)?mini:d;
					}else if(d>0) mini=(mini<d)?mini:d;
				}else if(c>0){
					mini=(mini<c)?mini:c;
					if(d>0) mini=(mini<d)?mini:d;
				}else if(d>0) mini=(mini<d)?mini:d;
			}else if(b>0){ //a=0 b>0
				mini=b;
				if(c>0){
					mini=(mini<c)?mini:c;
					if(d>0) mini=(mini<d)?mini:d;
				}else if(d>0) mini=(mini<d)?mini:d;
			}else if(c>0){ //a,b=0 c>0
				mini=c;
				if(d>0) mini=(mini<d)?mini:d;
			}else if(d>0) mini=d; //a,b,c=0 d>0
			if(X==255){
				if(maks==0){
					l++;
//					cout << l << endl;
					tab[i][j]=l;
					LUT[l][1]=l;
				}else{
					tab[i][j]=mini;
					if(mini!=maks) LUT[maks][1]=mini;
				}
			}
		}
	}

	//indeksacja 2 tura

	for(int i=1;i<width-1;i++){
		for(int j=1;j<height-1;j++){
			X=tab[i][j];
			if(X!=0)
				if(LUT[X][0]!=LUT[X][1]) tab[i][j]=LUT[X][1];
		}
	}

	// indeksacja narpawienie LUT
	int tmp1;
	for(int i=1;i<LUT_SIZE;i++){
		if(LUT[i][0]!=LUT[i][1]){
			tmp1=0;
			for(int j=i+1;j<LUT_SIZE;j++){
				if(LUT[j][0]==LUT[j][1]){
					tmp1=j;
					break;
				}
			}
			if(tmp1>0){
				LUT[tmp1][1]=i;
				break;
			}
		}
	}

	//indeksacja poprawka indeksow na obrazie

	for(int i=1;i<width-1;i++){
		for(int j=1;j<height-1;j++){
			X=tab[i][j];
			if(X!=0)
				if(LUT[X][0]!=LUT[X][1]) tab[i][j]=LUT[X][1];
		}
	}

	//czyszczenie LUT od teraz bedzie przechowywac pola!
	for(int i=0;i<LUT_SIZE;i++) LUT[i][1]=0;

	//liczymy pola i ilosc dziur
	ileDziur=0;
	for(int i=1;i<width-1;i++){
		for(int j=1;j<height-1;j++){
			X=tab[i][j];
			if(X!=0) LUT[X][1]++;
		}
	}

	//zbieramy informacje i ilosci dziur o odpowiedniej proporcji powierzchni do powierzchni dłoni
	// proporcja przechowywana jest w progP
	// mozna ją dynamicznie zmieniać podczas działąnia programu
	for(int i=1;i<=LUT_SIZE;i++){
		if((double)count/LUT[i][1]<(double)progP) ileDziur++;
	}

	//na podstawie ilości odpowiednich dziur wyznaczamy gest dłoni
	//i podejmujemy odpowiednie decyzje
	if(ileDziur==CALA_DLON){
		gestWykryty=CALA_DLON;
		cout<<"DLON ";
	}
	else if(ileDziur==PALEC){
		gestWykryty=PALEC;
		cout<<"PALEC ";
	}

	else if(ileDziur==PIESC){
		gestWykryty=PIESC;
		cout<<"PIESC ";
	}
	else{
		gestWykryty=INNY;
		cout<<"INNY ";
	}

	cout << endl;

	if(gestUstalony!=gestWykryty){
		if(gestWykryty==gestTmp){
			gestLicznik++;
		}
		else{
			gestTmp=gestWykryty;
			gestLicznik=0;
		}
	}
	else{
		gestLicznik=0;
	}
	if(gestLicznik==5) { //dopiero kiedy dany gest jest wykryty 5 razy pod rzad jest on brany pod uwage
		//jesli przechwytywanie jest wlaczone
		cout << "zatwierdzam nowy gest" << endl;
		if(ruch==1){
			if(gestUstalony==PALEC) {
				lewyPusc();
				cout << "lewy pusc" << endl;
			}
			else if(gestUstalony==PIESC){
				prawyPusc();
				cout<<"prawy pusc"<<endl;
			}
		}
		//aktualizacja gestu
		gestUstalony=gestWykryty;
		//jesli przechwytywanie jest wlaczone
		if(ruch==1){
			if(gestUstalony==PALEC){
				lewyNacisnij();
				cout<<"lewy nacisnij"<<endl;
			}
			else if(gestUstalony==PIESC){
				prawyNacisnij();
				cout<<"prawy nacisnij"<<endl;
			}
		}
	}

	cvCircle(T,cvPoint(xt,yt),3,cvScalar(128));

	cvShowImage("Ub",Ub);
	cvShowImage("Vb",Vb);
	cvShowImage("T",T);
}

void pru(int val){
	progU=val;
	binary();
}

void prv(int val){
	progV=val;
	binary();
}

void prp(int val){
	progP=val;
	binary();
}

int main(int argc, char *argv[], char *envp[]){
    //jeżeli nie chcesz korzystac z pliku zakomentuj ponizsza linijke:
	kamera=1;  sciezka = "kolo.AVI";

	printf("\nProgram kieruje myszką, sterowanie:\n\ts\tstart przechwytywania\n\tu\tprzerwa w przechwytywaniu na ustawienia\n\tp\t pauza w wykonywaniu programu, ponowne wcisniecie koniec pauzy\n\tk/ESC\tkoniec\n\n");

	progU=136;
	progV=0;
	progP=18;

	ruch=0;

	gestLicznik=0;
	gestUstalony=INNY;
	gestWykryty=INNY;
	gestTmp=INNY;

	CvCapture * film;
	if(kamera==1)film = cvCaptureFromFile(sciezka);
	else film = cvCaptureFromCAM(-1);

	//double fps = cvGetCaptureProperty(film,CV_CAP_PROP_FPS);
	//int fcount = (int) cvGetCaptureProperty(film,CV_CAP_PROP_FRAME_COUNT);

	width = (int) cvGetCaptureProperty(film,CV_CAP_PROP_FRAME_WIDTH);
	height = (int) cvGetCaptureProperty(film,CV_CAP_PROP_FRAME_HEIGHT);

	screenw=GetSystemMetrics(SM_CXSCREEN);
	screenh=GetSystemMetrics(SM_CYSCREEN);

	x=(int)width/2;
	y=(int)height/2;

	tab = (int**)malloc(width*sizeof(int*));

	for(int i =0;i<width;i++) tab[i]= (int*)malloc(height*sizeof(int));

	for(int i=0;i<LUT_SIZE;i++){
		LUT[i][0]=i;
		LUT[i][1]=0;
	}

	Y = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U, 1);
	U = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U, 1);
	V = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U, 1);
	Ub = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U, 1);
	Vb = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U, 1);
	B = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U, 1);
	T = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U, 1);
	tmp = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U, 1);
	cvNamedWindow("tools",CV_WINDOW_AUTOSIZE);
	cvResizeWindow("tools",300,220);
	cvCreateTrackbar("U","tools",&progU,255,pru);
	cvCreateTrackbar("V","tools",&progV,255,prv);
	cvCreateTrackbar("P","tools",&progP,30,prp);
	cvNamedWindow("main",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Ub",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Vb",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("T",CV_WINDOW_AUTOSIZE);

	//for(int f=0;f<fcount;f++){
	for(;;){
		//zczytanie klatki
		img=cvQueryFrame(film);

		//filtr medianowy
		for(int i=0;i<3;i++) cvSmooth(img,img,CV_MEDIAN,3,0,0,0);

		cvShowImage("main",img);



		// RGB -> YUV
		cvCvtColor(img,img,CV_BGR2YCrCb);

		//rozdzial na poszczegulne kanaly
		for(int i=0;i<height;i++)
			for(int j=0;j<width;j++){
				uchar *y = &CV_IMAGE_ELEM(Y,uchar,i,j);
				uchar *u = &CV_IMAGE_ELEM(U,uchar,i,j);
				uchar *v = &CV_IMAGE_ELEM(V,uchar,i,j);
				y[0]=(&CV_IMAGE_ELEM(img,uchar,i,j*3))[0];
				u[0]=(&CV_IMAGE_ELEM(img,uchar,i,j*3+1))[0];
				v[0]=(&CV_IMAGE_ELEM(img,uchar,i,j*3+2))[0];
			}

		//binaryzacja i cala reszta obrobki klatki
		binary();

		//prosty interfejs użytkownika
		if(ruch==1) SetCursorPos(x,y);

		int key=cvWaitKey(40);
		if(key=='u') ruch=0;
		if(key=='s') ruch=1;
		if(key=='k' || key==27) break;
		if(key=='p'){
			key=0;
			for(;;){
				key=cvWaitKey(40);
				if(key=='p') break;
			}
		}
	}
	for(int i =0;i<width;i++) free(tab[i]);
	free(tab);
	printf("\n\n\n\tnacisnij dowolny klawisz aby kontynuować...");
	cvWaitKey();
	cvDestroyAllWindows();
	return 0;
}
