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
    void OnExit(wxCommandEvent& e);
    wxDECLARE_EVENT_TABLE();

public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    bool work(std::string argv);
};