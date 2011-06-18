#include "SystemAPI.h"

void SystemAPI::pressLeft()
{
	INPUT input;
	ZeroMemory(&input,sizeof(INPUT));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1,&input,sizeof(INPUT));
}

void SystemAPI::releaseLeft()
{
	INPUT input;
	ZeroMemory(&input,sizeof(INPUT));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1,&input,sizeof(INPUT));
}

void SystemAPI::pressRight()
{
	INPUT input;
	ZeroMemory(&input,sizeof(INPUT));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	SendInput(1,&input,sizeof(INPUT));
}

void SystemAPI::releaseRight()
{
	INPUT input;
	ZeroMemory(&input,sizeof(INPUT));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	SendInput(1,&input,sizeof(INPUT));
}

void SystemAPI::move(int x, int y)
{
	POINT pos;
	ZeroMemory(&pos,sizeof(POINT));
	GetCursorPos(&pos);
	SetCursorPos(pos.x+x,pos.y+y);
}
