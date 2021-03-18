#include "app.hpp"

bool InjectorApp::OnInit()
{
    MainFrame* f = new MainFrame();
    f->Show(true);
    bool worked = false;
    if (this->argc >= 2)
        worked = f->work(this->argv[1]);
    else
        worked = f->work("");
    
    if (worked)
    {
        // close in 3 seconds
    }
    else
    {
        // never close, and display error
    }

    return true;
}

void MainFrame::OnExit(wxCommandEvent& e)
{
    this->Close(true);
}

void MainFrame::OnMouseDown(wxMouseEvent& e)
{
    if (this->Topbar.CloseHovered)
        this->Topbar.Active = 1;
    else if (this->Topbar.MinimizeHovered)
        this->Topbar.Active = 2;
    else
        this->Topbar.Active = 0; // should already be 0 but whatever

    if (this->Topbar.Active != 0)
        this->Refresh();

    wxPoint mpos = e.GetPosition();
    if (0 <= mpos.y && mpos.y < MainFrame::TopbarHeight)
    {
        if (0 <= mpos.x && mpos.x < MainFrame::TopbarDraggableWidth)
        {
            this->DragData.OriginalWindowPos = this->GetPosition();
            this->DragData.OriginalMousePos = mpos + this->DragData.OriginalWindowPos;
            this->DragData.Dragging = true;
        }
    }
}

void MainFrame::OnMouseUp(wxMouseEvent& e)
{
    this->DragData.Dragging = false;

    if (this->Topbar.CloseHovered && this->Topbar.Active == 1)
        this->Close(true);
    else if (this->Topbar.MinimizeHovered && this->Topbar.Active == 2)
        this->Iconize(true);

    if (this->Topbar.Active != 0)
    {
        this->Topbar.Active = 0;
        this->Refresh();
    }
}

void MainFrame::OnMouseMove(wxMouseEvent& e)
{
    wxPoint mpos = e.GetPosition();
    if (this->DragData.Dragging)
    {
        wxPoint Delta = mpos + this->GetPosition() - this->DragData.OriginalMousePos;
        this->SetPosition(this->DragData.OriginalWindowPos + Delta);
        return;
    }

    bool CloseHoverBefore = this->Topbar.CloseHovered;
    this->Topbar.CloseHovered =
        0 <= mpos.y && mpos.y < MainFrame::TopbarButtonSize&&
        MainFrame::Width - MainFrame::TopbarButtonSize <= mpos.x && mpos.x < MainFrame::Width;

    bool MinimizeHoverBefore = this->Topbar.MinimizeHovered;
    this->Topbar.MinimizeHovered =
        0 <= mpos.y && mpos.y < MainFrame::TopbarButtonSize&&
        MainFrame::Width - MainFrame::TopbarButtonSize * 2 <= mpos.x && mpos.x < MainFrame::Width - MainFrame::TopbarButtonSize;

    if (MinimizeHoverBefore != this->Topbar.MinimizeHovered || CloseHoverBefore != this->Topbar.CloseHovered)
        this->Refresh();
}

#define COL32(R, G, B, A) ((uint32_t)(R | (G << 8) | (B << 16) | (A << 24)))
void MainFrame::OnPaint(wxPaintEvent& e)
{
    constexpr static auto Topbar           = COL32( 20,  20,  20, 255);
    constexpr static auto Button           = COL32( 20,  20,  20, 255);
    constexpr static auto ButtonHovered    = COL32( 50,  50,  50, 255);
    constexpr static auto ButtonActive     = COL32(100, 100, 100, 255);
    constexpr static auto ButtonForeground = COL32(220, 220, 220, 255);
    constexpr static auto TextForeground   = COL32(120, 120, 120, 255);
    const static wxFont TextFont(wxFontInfo(10).FaceName("Arial").AntiAliased(true).Weight(700).Italic(true));

    // topbar background
    wxPaintDC dc(this);
    dc.SetBrush(wxBrush(Topbar));
    dc.SetPen(wxPen(Topbar, 0));
    dc.DrawRectangle(0, 0, MainFrame::Width, MainFrame::TopbarHeight);

    // topbar text
    dc.SetTextForeground(TextForeground);
    dc.SetFont(TextFont);
    dc.DrawText("A4G4 INJECTOR", wxPoint(7, (MainFrame::TopbarHeight - TextFont.GetPixelSize().y)/2));

    // topbar buttons
    { // close button
        int x = MainFrame::Width - MainFrame::TopbarButtonSize;
        wxColor BgCol = this->Topbar.Active == 2 ? Button : (this->Topbar.CloseHovered ? (this->Topbar.Active == 1 ? ButtonActive : ButtonHovered) : Button);
        dc.SetBrush(wxBrush(BgCol));
        dc.SetPen(wxPen(BgCol, 0));
        dc.DrawRectangle(x, 0, MainFrame::TopbarButtonSize, MainFrame::TopbarHeight);

        wxPoint center(x + MainFrame::TopbarButtonSize / 2, MainFrame::TopbarHeight / 2);
        dc.SetPen(wxPen(ButtonForeground, 1));
        dc.DrawLine(center + wxPoint(-4, -4), center + wxPoint(5, 5));
        dc.DrawLine(center + wxPoint(4, -4), center + wxPoint(-5, 5));
    }
    { // minimize button
        int x = MainFrame::Width - MainFrame::TopbarButtonSize * 2;
        wxColor BgCol = this->Topbar.Active == 1 ? Button : (this->Topbar.MinimizeHovered ? (this->Topbar.Active == 2 ? ButtonActive : ButtonHovered) : Button);
        dc.SetBrush(wxBrush(BgCol));
        dc.SetPen(wxPen(BgCol, 0));
        dc.DrawRectangle(x, 0, MainFrame::TopbarButtonSize, MainFrame::TopbarHeight);

        dc.SetPen(wxPen(ButtonForeground, 1));
        dc.DrawLine(wxPoint(x + 12, MainFrame::TopbarHeight / 2), wxPoint(x + MainFrame::TopbarButtonSize - 12, MainFrame::TopbarHeight / 2));
    }
}

#define WINDOW_STYLE (wxSYSTEM_MENU | wxBORDER_NONE | wxCLIP_CHILDREN)

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, "A4G4 Injector", wxPoint(200, 200), wxSize(MainFrame::Width, MainFrame::Height), WINDOW_STYLE)
{
    this->SetBackgroundColour(wxColor(60, 60, 60));
}

bool MainFrame::work(std::string argv)
{
    Update::init(argv);
    Update::run();
    return true;
}

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_LEFT_DOWN(MainFrame::OnMouseDown)
EVT_LEFT_UP(MainFrame::OnMouseUp)
EVT_PAINT(MainFrame::OnPaint)
EVT_MOTION(MainFrame::OnMouseMove)
wxEND_EVENT_TABLE()