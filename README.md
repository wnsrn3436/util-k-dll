# K-DLL

[한국어](README.ko.md)

A C++ DLL that lets GameMaker take Korean input. GameMaker only puts finished characters into `keyboard_string`, so Korean text does not show on screen while it is being typed. This DLL reads the characters under composition straight from the Windows IME. It also switches or reads the Korean and English input state from code, hides the IME composition box, and converts between CP949 and UTF-8. It consists of the DLL, a GameMaker extension package, and a chat box example.

<p>
  <img src="docs/screenshots/screenshot-1.png" width="316" alt="Example screen">
</p>


## How to use

The archive downloaded from Releases contains the GameMaker 8 extension package `k-dll.gex` and the example project. Install the extension package, include it in a project, and call `kdll_init()` once.

| Function | Action |
|---|---|
| `kdll_init()` | Finds the game window. Call it once before any other function |
| `ime_get_string_ansi()` | Returns the characters under composition as a CP949 string, or an empty string when nothing is being composed |
| `ime_get_string_utf8()` | Returns the characters under composition as a UTF-8 string |
| `ime_set_language(mode)` | Switches the input language. 0 is English and 1 is Korean. Any composition in progress is cancelled |
| `ime_get_language()` | Returns whether Korean input is currently on |
| `ime_set_composition_window(show)` | 0 hides the IME composition box and 1 shows it again |
| `encoding_ansi_to_utf8(str)` | Converts a CP949 string to UTF-8 |
| `encoding_utf8_to_ansi(str)` | Converts a UTF-8 string to CP949 |
| `ansi_ord_at(str, index)` | Returns the byte value at position `index` of the string. `index` starts at 1 |

For chat input, draw the characters under composition right after the confirmed text in `keyboard_string`.

```gml
sex_draw_text(16, 256, keyboard_string+ime_get_string_ansi())
```

The example is a 300x300 chat box. As you type, even the Korean still under composition shows right away, Enter adds the text to the log, and Backspace removes a whole Korean character. The top left shows the FPS and the Korean or English input state.


## How it works

**It finds the game window itself.** GameMaker does not hand a window handle to a DLL. `kdll_init` goes through the top-level windows one by one and keeps the one that belongs to the current process, and every function after that gets its IME context through that handle.

```cpp
hWnd=FindWindow(NULL, NULL);

while(hWnd!=NULL)
{
    if(GetParent(hWnd)==NULL)
    {
        GetWindowThreadProcessId(hWnd, &idProc);
        if(TempCurrProcId==idProc){break;}
    }
    hWnd=GetWindow(hWnd, GW_HWNDNEXT);
}
```

**Characters under composition are read from the IME.** The composition string comes from two calls to `ImmGetCompositionString`: one asks for the length and the other receives the content. The ANSI version returns it as is, and the Unicode version converts it to UTF-8 with `WideCharToMultiByte` before passing it on. Once a character is confirmed, GameMaker puts it into `keyboard_string`, so joining the two gives the sentence being typed.

**The composition box is hidden by moving it off the window.** There is no API that turns the IME composition box off, so `ImmSetCompositionWindow` moves the box 1000 pixels out from the top left of the window. The IME puts it back every time a composition starts, so a `WH_GETMESSAGE` hook moves it again whenever a `WM_IME_COMPOSITION` message arrives. To show the box again, the DLL removes the hook and puts the box back in its default position.

```cpp
case WM_IME_COMPOSITION:
    {
        if(hIMC=ImmGetContext(hWnd))
        {
            tagRECT tagRect;
            GetWindowRect(hWnd, &tagRect);
            COMPOSITIONFORM cf;
            cf.dwStyle=CFS_POINT;
            cf.ptCurrentPos.x=tagRect.left-1000;
            cf.ptCurrentPos.y=tagRect.top-1000;
            ImmSetCompositionWindow(hIMC, &cf);
            ImmReleaseContext(hWnd, hIMC);
        }
        break;
    }
```

**Encoding conversion goes through Unicode.** `MultiByteToWideChar` converts the string to Unicode, and `WideCharToMultiByte` converts it from there to the target encoding. The result is returned in a buffer inside the DLL, which is freed on the next call.


## Files

| Path | Content |
|---|---|
| `source/ime.cpp`, `source/export.h` | DLL source |
| `source/KDLL.dev`, `source/Makefile.win` | wxDev-C++ project and MinGW makefile |
| `docs/screenshots/` | Screenshots |
| Releases | Extension package and example |


## Credits

The Korean text output script (Sefx) in the example was made by 김게맛 (sodium031) of the GameMaker community.


## License

zlib. See [LICENSE](LICENSE). Bundled libraries made by other people keep their own licenses.
