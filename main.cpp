#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <windows.h>

#include <mmsystem.h> // timeGetTime()�̂���
#pragma comment(lib, "winmm.lib") // VC++�̏ꍇ�A����ł������N�\

typedef struct VECTOR {
	VECTOR() {
		x = 0;
		y = 0;
	}
	VECTOR(float x, float y) {
		this->x = x;
		this->y = y;
	}
	VECTOR operator + (VECTOR& v) const {
		return VECTOR(x + v.x, y + v.y);
	}
	VECTOR operator - (VECTOR& v) const {
		return VECTOR(x - v.x, y - v.y);
	}
	VECTOR operator * (const float v) {
		return VECTOR(x * v, y * v);
	}
	VECTOR operator / (const float v) {
		return VECTOR(x / v, y / v);
	}
	float dot(VECTOR& v) {
		return x * v.x + y * v.y;
	}
	float sq_length() {
		return dot(*this);
	}
	float length() {
		return sqrtf(sq_length());
	}
	VECTOR normalize() {
		float v = length();
		return *this / v;
	}
	float x;
	float y;
}VECTOR;

typedef struct LINE{
	LINE(){
		start.x = 0;
		start.y = 0;
		end.x = 0;
		end.y = 0;
	}
	LINE(float sx, float sy, float ex, float ey){
		start.x = sx;
		start.y = sy;
		end.x = ex;
		end.y = ey;
	}
	static LINE* alloc(float sx, float sy, float ex, float ey) {
		return new LINE(sx, sy, ex, ey);
	}
	void set(float sx, float sy, float ex, float ey) {
		start.x = sx;
		start.y = sy;
		end.x = ex;
		end.y = ey;
	}
	void release() {
		delete this;
	}
	VECTOR start;
	VECTOR end;
}LINE;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);       // �E�B���h�E�v���V�[�W��

void gameMain(void); // �Q�[�����C������
void wait(DWORD); // �E�F�C�g

HWND g_hWnd = NULL;

const DWORD FPS = 60; // FPS�ݒ�
BOOL EndFlag = FALSE; // �I���t���O

VECTOR mouse_pos;

/**
 * Windows ���C������
 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){

	// �A�v���P�[�V�����̏�����
	HWND hWnd;
	WNDCLASSEX wcex ={sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, NULL, LoadCursor(NULL , IDC_ARROW),
	(HBRUSH)(COLOR_WINDOW+1), NULL, "cls_name", NULL};

	if(!RegisterClassEx(&wcex))
		return 0;

	int w = 640, h = 480;
	RECT clientRect = {0, 0, w, h};
	::AdjustWindowRect( &clientRect, WS_OVERLAPPEDWINDOW, FALSE );

	if(!(hWnd = CreateWindow("cls_name", "�C���C���_", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
		clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,
		NULL, NULL, hInstance, NULL)))
		return 0;

	ShowWindow(hWnd, nCmdShow);
	g_hWnd = hWnd;

	// �Q�[�����C�������ց`
	gameMain();

	return 0; // �Ƃ肠����0��Ԃ�
}

/**
 * �E�B���h�E�v���V�[�W��
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){

	switch(msg){
		case WM_CREATE:

			return 0;
		case WM_KEYDOWN:
			if(wParam == VK_ESCAPE)
				DestroyWindow(hWnd);
			return 0;
		case WM_MOUSEMOVE:
			mouse_pos.x = (float)LOWORD(lParam);
			mouse_pos.y = (float)HIWORD(lParam);
			return 0;
		case WM_DESTROY:                                    // �E�B���h�E���j�����ꂽ�Ƃ��̏���
			PostQuitMessage(0); // �I�����b�Z�[�W
			EndFlag = TRUE; // �I���t���O��������B
			return 0;


		default:                                            // �f�t�H���g����
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

/**
 * �E�F�C�g
 * @param wait_time �҂���
 */
void wait(DWORD wait_time){
	MSG msg;
	DWORD start_time = timeGetTime();

	do{
		// ���b�Z�[�W����
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(wait_time > 0) Sleep(1); // ������Ƌx�e�iCPU�̐�L���������邽�߁j
	}while(timeGetTime() < wait_time + start_time); // wait_time �������
}

/**
 * �����`��
 * @param hdc �f�o�C�X�R���e�L�X�g
 * @param str �`�敶����
 * @param x ���WX
 * @param y ���WY
 */
void drawString(HDC hdc, std::string str, float x, float y) {
	TextOut(hdc, (int)x, (int)y, str.c_str(), str.length());
}

HDC createEmptyBMP(HDC hdc, int width, int height){
    HBITMAP hbmp;
    HDC hdc_work;

    hbmp = CreateCompatibleBitmap(hdc, width, height);
    hdc_work = CreateCompatibleDC(hdc);
    SelectObject(hdc_work, hbmp);
    PatBlt(hdc_work, 0, 0, width, height, WHITENESS); // ���œh��Ԃ�
    DeleteObject(hbmp);
    return hdc_work;
}

/*
 * �Q�[�����C������
 */
void gameMain(void){

	DWORD StartTime, EndTime, PassTime;

	PAINTSTRUCT ps;
	HDC hdc;

	VECTOR p(120, 130);

	std::vector<LINE*> lines;

	lines.push_back(LINE::alloc( 40,  40, 400,  40));
	lines.push_back(LINE::alloc(400,  40, 480, 120));
	lines.push_back(LINE::alloc(480, 120, 480, 440));
	lines.push_back(LINE::alloc(480, 440,  40, 440));
	lines.push_back(LINE::alloc( 40, 440,  40, 140));
	lines.push_back(LINE::alloc( 40, 140, 300, 240));
	lines.push_back(LINE::alloc(300, 240, 300, 340));
	lines.push_back(LINE::alloc(300, 340, 120, 240));
	lines.push_back(LINE::alloc(120, 240, 120, 380));
	lines.push_back(LINE::alloc(120, 380, 400, 380));
	lines.push_back(LINE::alloc(400, 380, 400, 180));
	lines.push_back(LINE::alloc(400, 180, 340, 120));
	lines.push_back(LINE::alloc(340, 120,  40, 120));
	lines.push_back(LINE::alloc( 40, 120,  40,  40));

	float speed = 2;

	int game_state = 0;
	hdc = BeginPaint(g_hWnd, &ps);
	HDC back_hdc = createEmptyBMP(hdc, 640, 480);
	int goal_after_time = 0;

	//���C�����[�v
	while(!EndFlag) {
		StartTime = timeGetTime();

		// �` �Q�[���������낢�� �`
		/*
		if(GetAsyncKeyState(VK_LEFT)) {
			p.x-=speed;
		}
		if(GetAsyncKeyState(VK_RIGHT)) {
			p.x+=speed;
		}
		if(GetAsyncKeyState(VK_UP)) {
			p.y-=speed;
		}
		if(GetAsyncKeyState(VK_DOWN)) {
			p.y+=speed;
		}
		*/

		switch(game_state) {
			case 0:
				p.x = 80;
				p.y = 80;
				if((p - mouse_pos).sq_length() < 15 * 15) {
					game_state = 1;
					ShowCursor(false);
				}
				break;
			case 1:
				p = mouse_pos;
				if((p - VECTOR(265, 275)).sq_length() < 30 * 30) {
					goal_after_time = 0;
					game_state = 2;
					ShowCursor(true);
				}
				break;

			case 2:
				goal_after_time++;
				if(goal_after_time >= 100) {
					game_state = 0;
				}
				break;
		}

		HPEN hpen;
		PatBlt(back_hdc, 0, 0, 640, 480, WHITENESS);

		for each(LINE* line in lines) {

			LINE* hit = NULL;
			float radius = 15;
			float radius2 = radius * radius;
			VECTOR tmp;
			VECTOR push_len;

			// �g���܂킵����̂ł����Ōv�Z���Ă���
			tmp = line->end - line->start;
			VECTOR pts(p - line->start);
			float t = (pts.dot(tmp)) / tmp.sq_length();

			if(0 <= t && t <= 1) {
				tmp = tmp * t + line->start;
				tmp = tmp - p;
				if(tmp.sq_length() < radius2) {
					hit = line;
				}
			}

			// line vs p

			if(!hit) {
				// start vs p
				tmp = line->start - p;
				if(tmp.sq_length() < radius2) {
					hit = line;

				} else {
					// end vs p
					tmp = line->end - p;
					if(tmp.sq_length() < radius2) {
						hit = line;
					}
				}
			}

			// �{���͂����ɂ�����A�����ΐ����̃q�b�g������K�v(�����ɓ�������(�e�e�Ƃ�)vs���������鎞)�B�O�ς�p����זʓ|�Ȃ̂ŏȂ�

			if(hit == line) {
				hpen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
				SelectObject(back_hdc, hpen);
				if(game_state == 1) ShowCursor(true);
				game_state = 0;
			}

			MoveToEx(back_hdc, (int)line->start.x, (int)line->start.y, NULL);
			LineTo(back_hdc, (int)line->end.x, (int)line->end.y);

			// nil����Ȃ��Ă�������ˁH
			if(hit == line) {
				DeleteObject(hpen);
				SelectObject(back_hdc, (HPEN)GetStockObject(BLACK_PEN));
			}

		}

		Ellipse(back_hdc, (int)p.x - 15, (int)p.y - 15, (int)p.x + 15, (int)p.y + 15);
		Ellipse(back_hdc, 250, 260, 280, 290);

		if(game_state == 0) {
			drawString(back_hdc, "�J�[�\�������킹��ƃX�^�[�g", 60, 50);
		}
		drawString(back_hdc, "�S�[��", 250, 240);

		if(game_state == 2) {
			drawString(back_hdc, "�R���O���`�����[�V����", 230, 230);
		}
		drawString(back_hdc, "ESC: �E�B���h�E�����", 0, 0);

		BitBlt(hdc, 0, 0, 640, 480, back_hdc, 0, 0, SRCCOPY);

		EndTime = timeGetTime();
		PassTime = EndTime - StartTime; // �o�ߎ��Ԃ̌v�Z

		(1000 / FPS > PassTime)? wait(1000 / FPS - PassTime) : wait(0); // �҂B
	}

	EndPaint(g_hWnd, &ps);

	for each(LINE* line in lines) {
		line->release();
	}

}