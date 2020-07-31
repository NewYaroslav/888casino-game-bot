#ifndef CASIO_DRAW_HPP_INCLUDED
#define CASIO_DRAW_HPP_INCLUDED

#include <windows.h>
#include <windowsx.h>

namespace casino {

    /** \brief Класс для рисования прицела поверх всех окон
     */
    class DrawTop {
    private:
        int x = 0;          /**< Позиция окна */
        int y = 0;          /**< Позиция окна */
        int width = 0;      /**< Ширина окна */
        int height = 0;     /**< Высота окна */
        HWND hwnd_image;
        HINSTANCE  hinst_image;
        std::string class_image;
        bool is_init = false;

        HBITMAP maskBitmap;
    public:

        static LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
            DrawTop* pThis;
            if(msg == WM_CREATE) {
                pThis = (DrawTop*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
                SetLastError(0);
                if (!SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis))) {
                    if (GetLastError() != 0) {
                        MessageBox(NULL, "You messed up.", "Error!", MB_ICONEXCLAMATION | MB_OK);
                        std::cout << "You messed up. Error!" << std::endl;
                        return FALSE;
                    }
                }

            } else {
                pThis = reinterpret_cast<DrawTop*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            }

            switch (msg) {
                case WM_CREATE: {
                        std::cout << "WM_CREATE" << std::endl;
                    }
                    break;
                case WM_PAINT: {
                        std::cout << "WM_PAINT" << std::endl;
                        PAINTSTRUCT ps;
                        HDC hdc = BeginPaint (hwnd, &ps);
                        HDC hdcBits = CreateCompatibleDC (hdc);
                        SelectObject (hdcBits, pThis->maskBitmap);
                        BitBlt (hdc, 0, 0, pThis->width, pThis->height, hdcBits, 0, 0, SRCCOPY);
                        DeleteDC (hdcBits);
                        EndPaint (hwnd, &ps);
                    }
                    break;
                case WM_CLOSE:
                    //DestroyWindow(hwnd);
                    std::cout << "WM_CLOSE" << std::endl;
                    break;
                case WM_DESTROY:
                    //CloseWindow(hwnd);
                    std::cout << "WM_DESTROY" << std::endl;
                    PostQuitMessage (0);
                    break;
            }
            return DefWindowProc (hwnd, msg, wParam, lParam);
        }

        DrawTop() {};

        DrawTop(HINSTANCE hInst,
                int nShowCmd,
                const std::string &image_file,
                const std::string &window_name,
                const int window_width,
                const int window_heigh,
                const int window_x = 0,
                const int window_y = 0) :
                hinst_image(hInst),
                width(window_width),
                height(window_heigh),
                x(window_x), y(window_y)  {
            class_image = window_name + "_Class";

            WNDCLASS wc      = {0};
            ZeroMemory (&wc, sizeof (wc));

            wc.style         = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc   = WindowProcedure;
            wc.hInstance     = hInst;
            wc.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
            wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
            wc.hbrBackground = CreateSolidBrush (RGB (150, 150, 150));
            wc.lpszClassName = class_image.c_str();
            wc.lpszMenuName  = NULL;
            wc.cbClsExtra    = 0;
            wc.cbWndExtra    = 0;

            if(!RegisterClass (&wc)) {
                 MessageBox (NULL, "Window Registration Failed!", "Error!", MB_OK | MB_ICONERROR);
                 return;
            }

            HWND hwnd_image = CreateWindowEx (WS_EX_TOPMOST, class_image.c_str(), window_name.c_str(), WS_OVERLAPPEDWINDOW, x, y, width, height, NULL, NULL, hInst, this);
            if (!hwnd_image) {
                 MessageBox (NULL, "Window Creation Failed!", "Error!", MB_OK | MB_ICONERROR);
                 return;
            }

            if(!SetWindowLong (hwnd_image, GWL_STYLE, GetWindowLong (hwnd_image, GWL_STYLE) || WS_CAPTION || WS_SYSMENU)) {
                MessageBox (NULL, "SetWindowLong Failed!", "Error!", MB_OK | MB_ICONERROR);
                return;
            }
/*
            ShowWindow (hwnd_image, nShowCmd);
            ShowWindow (hwnd_image, SW_SHOW);

            if(!UpdateWindow (hwnd_image)) {
                MessageBox (NULL, "UpdateWindow Failed!", "Error!", MB_OK | MB_ICONERROR);
                return;
            }
*/
            maskBitmap = (HBITMAP) LoadImage (NULL, image_file.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            if (!maskBitmap) {
                 MessageBox (NULL, "Image not found!", "Error!", MB_OK | MB_ICONERROR);
                 return;
            }

            BITMAP bi;
            int startx;

            HRGN Rgn, ResRgn = CreateRectRgn (0, 0, 0, 0);
            GetObject (maskBitmap, sizeof (BITMAP), &bi);

            BYTE bpp = bi.bmBitsPixel >> 3;
            BYTE *pBits = new BYTE[ bi.bmWidth * bi.bmHeight * bpp ];

            int p = GetBitmapBits (maskBitmap, bi.bmWidth * bi.bmHeight * bpp, pBits);

            DWORD TransPixel = *(DWORD*) pBits;
            TransPixel <<= 32 - bi.bmBitsPixel;

            DWORD pixel;
            for (INT i = 0; i < bi.bmHeight; i ++) {
                 startx =- 1;
                 for (INT j = 0; j < bi.bmWidth; j ++) {
                      pixel = *(DWORD*)(pBits + (i * bi.bmWidth + j) * bpp) << (32 - bi.bmBitsPixel);
                      if (pixel != TransPixel) {
                           if (startx < 0) {
                                startx = j;
                           } else if (j == (bi.bmWidth - 1)) {
                                Rgn = CreateRectRgn (startx, i, j, i + 1);
                            CombineRgn (ResRgn, ResRgn, Rgn, RGN_OR);
                            startx =- 1;
                           }
                      } else if (startx >= 0) {
                           Rgn = CreateRectRgn (startx, i, j, i + 1);
                           CombineRgn (ResRgn, ResRgn, Rgn, RGN_OR);
                           startx =- 1;
                      }
                 }
            }
            delete pBits;

            SetWindowRgn (hwnd_image, ResRgn, TRUE);
            InvalidateRect (hwnd_image, 0, false);

            ShowWindow (hwnd_image, nShowCmd);
            ShowWindow (hwnd_image, SW_SHOW);

            if(!UpdateWindow (hwnd_image)) {
                MessageBox (NULL, "UpdateWindow Failed!", "Error!", MB_OK | MB_ICONERROR);
                return;
            }

            is_init = true;
        };

        ~DrawTop() {
            //SendMessage(hwnd_image, WM_NCDESTROY, 0, 0);
            SendMessage(hwnd_image, WM_SYSCOMMAND, SC_CLOSE, 0);
            for(int i = 0; i < 5; ++i) {
                update();
            }
            //ShowWindow (hwnd_image, SW_HIDE);
            //UpdateWindow (hwnd_image);
            //CloseWindow(hwnd_image);
            //DestroyWindow(hwnd_image);
            //UnregisterClass (class_image.c_str(), hinst_image);
        };

        static bool update() {
            MSG msg;
            if(GetMessage (&msg, NULL, 0, 0)) {
                DispatchMessage(&msg);
                TranslateMessage(&msg);
                return true;
            }
            return false;
        }

        bool set_pos(const int x, const int y) {
            //if(!is_init) return false;
            MoveWindow(hwnd_image, x, y, width, height, FALSE);
            UpdateWindow (hwnd_image);
            return true;
        }
    };
}
#endif // CASIO_DRAW_HPP_INCLUDED
