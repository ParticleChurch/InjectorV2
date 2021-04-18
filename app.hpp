#pragma once
#include <wx/wx.h>
#include <wx/dcgraph.h>
#include <wx/dcbuffer.h>
#include "HTTP.hpp"
#include "update.hpp"
#include <Windows.h>
#include <TlHelp32.h>
#include <psapi.h>
#include <string>
#include <chrono>
#include "ManualMapper.hpp"
#include "Encryption.hpp"
#define TIME_POINT std::chrono::steady_clock::time_point
#define TIME_NOW() (std::chrono::steady_clock::now())
#define TIME_DIFF(after, before) (std::chrono::duration_cast<std::chrono::microseconds>((after) - (before)).count() / (double)1e+6)
#define ERROR_STR(context, code) (std::to_string((int)(context)) + "-" + std::to_string((int)(code)) + "v" + std::string(INJECTOR_CURRENT_VERSION))

class InjectorApp : public wxApp
{
public:
	virtual bool OnInit();
};

class MainFrame;
struct Step
{
    std::string Title = "Loading...";
    std::string State = "Waiting...";
    float Progress = 0.f;
    bool Failed = false;
    bool Active = false;
    TIME_POINT TimeBegan = TIME_NOW();
    bool (*handler)(MainFrame*);

    Step(std::string title) : Title(title) {}
};

class MainWorker : public wxThread
{
    wxFrame* parent;
public:
    MainWorker(wxFrame* parent);
    void* Entry();
};

class PeriodicPainter : public wxThread
{
    wxFrame* parent;
    int timeIntervalMS = 0;
public:
    PeriodicPainter(wxFrame* parent, int ms);
    void* Entry();
};

class MainFrame : public wxFrame
{
public:
    constexpr static int Width = 400;

    constexpr static int TopbarHeight = 23;
    constexpr static int TopbarButtonSize = 32;
    constexpr static int TopbarDraggableWidth = Width - TopbarButtonSize * 2;

    constexpr static size_t StepCount = 4;
    constexpr static int StepListingSize = 50;
    constexpr static int StepListingPadding = 0;
    constexpr static int StepLoadingRadius = 13;
    constexpr static int StepLoadingCirclePadding = StepListingSize / 2 - StepLoadingRadius;

    constexpr static int Height = TopbarHeight + (StepListingPadding + StepListingSize) * StepCount + StepListingPadding;

    Encryption::Header DLLHeader;
    char* DLLFile = nullptr;
    size_t DLLFileSize = 0;
    HANDLE CSGO = nullptr;
    DWORD CSGO_PID = 0;
    size_t numDllsLoaded = 0;
    size_t totalDllsToLoad = 1;
    Step Steps[StepCount] = {
        Step("Checking for Injector Updates"),
        Step("Finding CS:GO Process"),
        Step("Downloading Latest DLL Version"),
        Step("Decrypting and Injecting")
    };
    MainWorker* worker;

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
    void OnPaint(wxPaintEvent& e); PeriodicPainter* repainter = new PeriodicPainter(this, 1);
    void OnMouseDown(wxMouseEvent& e);
    void OnMouseUp(wxMouseEvent& e);
    void OnMouseMove(wxMouseEvent& e);
    void OnLeaveWindow(wxMouseEvent& e);
    void OnEraseBackground(wxEraseEvent& e);
    wxDECLARE_EVENT_TABLE();
public:

    MainFrame();

    bool work(std::string argv);
    bool HandleInjectorUpdateStep();
    bool HandleFindCSGOStep();
    bool HandleDownloadStep();
    bool HandleInjectStep();
    void Exit();
};