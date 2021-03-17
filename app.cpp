#include "app.hpp"

bool InjectorApp::OnInit()
{
    MainFrame* f = new MainFrame("Hello World", wxPoint(0, 0), wxSize(450, 340));
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
}

void MainFrame::OnExit(wxCommandEvent& e)
{
    this->Close(true);
}

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame(NULL, wxID_ANY, title, pos, size)
{

}

bool MainFrame::work(std::string argv)
{
    Update::init(argv);
    Update::run();
    return true;
}

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
wxEND_EVENT_TABLE()