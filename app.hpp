#pragma once
#include <wx/wx.h>
#include "HTTP.hpp"
#include "update.hpp"

class InjectorApp : public wxApp
{
public:
	virtual bool OnInit();
};

class MainFrame : public wxFrame
{
    constexpr static int Width = 400;
    constexpr static int Height = 300;

    constexpr static int TopbarHeight = 23;
    constexpr static int TopbarButtonSize = 32;
    constexpr static int TopbarDraggableWidth = Width - TopbarButtonSize * 2;

    struct {
        bool Dragging = false;
        wxPoint OriginalMousePos; // globally
        wxPoint OriginalWindowPos; 
    } DragData;

    struct {
        bool CloseHovered = false;
        bool MinimizeHovered = false;
        int Active = 0; // 0 = none, 1 = close, 2 = minimize
    } Topbar;

    void OnExit(wxCommandEvent& e);
    void OnPaint(wxPaintEvent& e);
    void OnMouseDown(wxMouseEvent& e);
    void OnMouseUp(wxMouseEvent& e);
    void OnMouseMove(wxMouseEvent& e);
    wxDECLARE_EVENT_TABLE();

public:
    MainFrame();

    bool work(std::string argv);
};