#include "app.hpp"

std::vector<std::string> CSGODLLs{
    "csgo.exe",
    "ntdll.dll",
    "engine.dll",
    "tier0.dll",
    "client.dll",
    "server.dll",
    "shaderapidx9.dll",
    "vguimatsurface.dll",
    "vgui2.dll",
    "vphysics.dll",
    "inputsystem.dll",
    "vstdlib.dll",
    "studiorender.dll",
    "materialsystem.dll",
    "serverbrowser.dll",
};

bool InjectorApp::OnInit()
{
    MainFrame* f = new MainFrame();
    f->Show(true);
    bool worked = false;
    if (this->argc >= 2)
        worked = f->work(this->argv[1]);
    else
        worked = f->work("");
    
    if (!worked)
    {
        f->Steps[0].Failed = true;
        f->Steps[0].State = "Update failed: " + ERROR_STR(Update::ErrorContext, Update::ErrorCode);

        for (int x = 1; x < MainFrame::StepCount; x++) f->Steps[x].State = "Previous step failed.";
    }
    return true;
}

#define WINDOW_STYLE (wxSYSTEM_MENU | wxBORDER_NONE | wxCLIP_CHILDREN)

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, "A4G4 Injector", wxPoint(200, 200), wxSize(MainFrame::Width, MainFrame::Height), WINDOW_STYLE)
{
    this->SetBackgroundColour(wxColor(60, 60, 60));
    this->repainter->Run();
}

void MainFrame::OnExit(wxCommandEvent& e)
{
    this->Exit();
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
        this->Exit();
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

void MainFrame::OnLeaveWindow(wxMouseEvent& e)
{
    this->Topbar.CloseHovered = false;
    this->Topbar.MinimizeHovered = false;
    this->Topbar.Active = 0;
    this->Refresh();
}

void MainFrame::OnEraseBackground(wxEraseEvent& e)
{
    //bruh
}

MainWorker::MainWorker(wxFrame* parent)
{
    this->parent = parent;
}

PeriodicPainter::PeriodicPainter(wxFrame* parent, int ms)
{
    this->parent = parent;
    this->timeIntervalMS = ms;
}

void* PeriodicPainter::Entry()
{
    while (!this->TestDestroy())
    {
        this->parent->Refresh();
        Sleep(this->timeIntervalMS);
    };
    return this;
}

#define COL32(R, G, B, A) ((uint32_t)(R | (G << 8) | (B << 16) | (A << 24)))
void MainFrame::OnPaint(wxPaintEvent& e)
{
    constexpr static auto Topbar             = COL32( 20,  20,  20, 255);
    constexpr static auto Button             = COL32( 20,  20,  20, 255);
    constexpr static auto ButtonHovered      = COL32( 50,  50,  50, 255);
    constexpr static auto ButtonActive       = COL32(100, 100, 100, 255);
    constexpr static auto ButtonForeground   = COL32(220, 220, 220, 255);
    constexpr static auto StepTitleText      = COL32(255, 255, 255, 255);
    constexpr static auto StepSubtitleText   = COL32(150, 150, 150, 255);
    constexpr static auto TextForeground     = COL32(120, 120, 120, 255);
    constexpr static auto ProgressForeground = COL32(255, 255, 255, 255);
    constexpr static auto WindowBg           = COL32( 60,  60,  60, 255);
    const static wxFont TextFont(wxFontInfo(10).FaceName("Arial").AntiAliased(true).Weight(700).Italic(true));

    // topbar background
    wxBufferedPaintDC dc(this);
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

    // window background, otherwise transparent
    {
        dc.SetBrush(wxBrush(WindowBg));
        dc.SetPen(wxPenInfo(COL32(255, 255, 255, 0), 0, wxPENSTYLE_TRANSPARENT));

        dc.DrawRectangle(0, MainFrame::TopbarHeight, MainFrame::Width, MainFrame::Height - MainFrame::TopbarHeight);
    }

    // steps progress
    {
        TIME_POINT now = TIME_NOW();
        bool foundActive = false;

        wxGCDC gdc(dc);
        gdc.SetPen(wxPen(ProgressForeground, 2));
        gdc.SetBrush(wxBrush(COL32(60, 60, 60 ,0)));
        for (size_t i = 0; i < this->StepCount; i++)
        {
            Step s = this->Steps[i];
            
            int y = MainFrame::TopbarHeight + MainFrame::StepListingPadding * (i + 1) + MainFrame::StepListingSize * i;
            int cx = MainFrame::StepLoadingCirclePadding + MainFrame::StepListingPadding + MainFrame::StepLoadingRadius;
            int cy = y + MainFrame::StepListingSize / 2;

            bool active = !foundActive && s.Active;
            foundActive |= active || s.Failed;

            if (active)
            {
                // currently working
                double time = TIME_DIFF(now, s.TimeBegan);
                double angleOffset = time * -200;
                gdc.DrawEllipticArc(
                    MainFrame::StepLoadingCirclePadding + MainFrame::StepListingPadding,
                    y + MainFrame::StepLoadingCirclePadding,
                    MainFrame::StepLoadingRadius * 2 - 2, MainFrame::StepLoadingRadius * 2 - 2,
                    0 + angleOffset, 90 + angleOffset
                );
                gdc.DrawEllipticArc(
                    MainFrame::StepLoadingCirclePadding + MainFrame::StepListingPadding,
                    y + MainFrame::StepLoadingCirclePadding,
                    MainFrame::StepLoadingRadius * 2 - 2, MainFrame::StepLoadingRadius * 2 - 2,
                    180 + angleOffset, 270 + angleOffset
                );
            }
            else if (s.Failed)
            {
                // errored
                gdc.SetPen(wxPen(ProgressForeground, 0, wxPENSTYLE_TRANSPARENT));
                gdc.SetBrush(wxBrush(COL32(235, 100, 100, 255)));
                gdc.DrawCircle(
                    cx,
                    cy,
                    MainFrame::StepLoadingRadius
                );
                gdc.SetPen(wxPen(ProgressForeground, 2));
                gdc.SetBrush(wxBrush(COL32(60, 60, 60, 0)));

                constexpr int off = 5;
                gdc.DrawLine(cx - off, cy - off, cx + off, cy + off);
                gdc.DrawLine(cx - off, cy + off, cx + off, cy - off);
            }
            else if (!foundActive)
            {
                // succeeded
                gdc.SetPen(wxPen(ProgressForeground, 0, wxPENSTYLE_TRANSPARENT));
                gdc.SetBrush(wxBrush(COL32(75, 160, 220, 255)));
                gdc.DrawCircle(
                    cx,
                    cy,
                    MainFrame::StepLoadingRadius
                );
                gdc.SetPen(wxPen(ProgressForeground, 2));
                gdc.SetBrush(wxBrush(COL32(60, 60, 60, 0)));

                gdc.DrawLine(cx - 5, cy, cx - 1, cy + 5);
                gdc.DrawLine(cx - 1, cy + 5, cx + 6, cy - 4);
            }
            else
            {
                // not yet started
                gdc.DrawCircle(
                    cx,
                    cy,
                    MainFrame::StepLoadingRadius - 1
                );
            }

            int TextX = cx * 2;

            // title
            static wxFont TitleFont = wxFontInfo(12).Weight(700).AntiAliased(true).Family(wxFONTFAMILY_DEFAULT).FaceName("Arial");
            static wxFont SubtitleFont = wxFontInfo(10).Weight(400).AntiAliased(true).Italic(true).Family(wxFONTFAMILY_DEFAULT).FaceName("Arial");

            gdc.SetFont(TitleFont);
            gdc.SetTextForeground(StepTitleText);
            gdc.DrawText(s.Title, TextX, y + 8);

            gdc.SetFont(SubtitleFont);
            gdc.SetTextForeground(StepSubtitleText);
            gdc.DrawText(s.State, TextX, y + 27);
        }
    }
}

void* MainWorker::Entry()
{
    MainFrame* p = (MainFrame*)this->parent;
    for (size_t i = 0; i < p->StepCount; i++)
    {
        p->Steps[i].Active = true;
        p->Steps[i].TimeBegan = TIME_NOW();
        p->Steps[i].Failed = !p->Steps[i].handler(p);
        p->Steps[i].Active = false;
        if (p->Steps[i].Failed)
            while (1) Sleep(10000);
        else
            p->Steps[i].State = "Completed";
    }

    p->Steps[MainFrame::StepCount - 1].State = "Success! Closing in 3...";
    Sleep(1000);
    p->Steps[MainFrame::StepCount - 1].State = "Success! Closing in 2...";
    Sleep(1000);
    p->Steps[MainFrame::StepCount - 1].State = "Success! Closing in 1...";

    p->repainter->Delete();

    Sleep(1000);
    std::exit(0);

    return this;
}

bool MainFrame::work(std::string argv)
{
    if (!Update::init(argv)) return false;

    this->Steps[0].handler = [](MainFrame* self) { return self->HandleInjectorUpdateStep(); };
    this->Steps[1].handler = [](MainFrame* self) { return self->HandleFindCSGOStep(); };
    this->Steps[2].handler = [](MainFrame* self) { return self->HandleDownloadStep(); };
    this->Steps[3].handler = [](MainFrame* self) { return self->HandleInjectStep(); };

    auto worker = new MainWorker(this);
    worker->Run();
    return true;
}

bool MainFrame::HandleInjectorUpdateStep()
{
    Step* s = this->Steps + 0;

    // check if we need to update
    switch (Update::versionCheck())
    {
    case Update::VersionCheckResult::Error:
        s->State = "Failed to check version - check your firewall (" + ERROR_STR(1, GetLastError()) + ")";
        return false;
    case Update::VersionCheckResult::UpToDate:
        s->State = "Up to date.";
        return true;
    default:
        // we need to update
        break;
    }

    // download new version
    s->State = "Downloading new version...";
    size_t newVersionSize = 0;
    char* newVersionFile = Update::downloadLatestVersion(&newVersionSize);
    if (!newVersionFile || newVersionSize == 0)
        return false;

    // rename myself to a temp filename
    s->State = "Performing update...";
    Update::TempFileName = Update::GenerateTempFileName(Update::Directory);
    if (!Update::renameMyself(Update::TempFileName))
    {
        s->State = "Failed to perform update - " + ERROR_STR(1, 1);
        return false;
    }

    // write new version to file
    Update::writeNewVersion(newVersionFile, newVersionSize);

    s->State = "Running new version...";
    if (!Update::executeNewVersion())
    {
        s->State = "Failed to execute new version - " + ERROR_STR(1,1);
        return false;
    }

    std::exit(1);
    return true;
}

bool processIsCSGO(HANDLE hProcess)
{
    if (!hProcess) return false;

    wchar_t _FileName[MAX_PATH + 1];
    GetModuleFileNameEx(hProcess, 0, _FileName, MAX_PATH);
    std::wstring FileName(_FileName);

    if (FileName.length() < strlen("csgo.exe")) return false;
    if (FileName.substr(FileName.size() - strlen("csgo.exe")) != L"csgo.exe") return false;
    return true;
}

HANDLE getCSGO()
{
    HANDLE proc;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(entry);
    do
    {
        proc = OpenProcess(PROCESS_ALL_ACCESS, false, entry.th32ProcessID);
        if (processIsCSGO(proc))
            return proc;
    } while (Process32Next(snapshot, &entry));
    CloseHandle(snapshot);

    return 0;
}

bool csgoIsInitialized(HANDLE csgo, DWORD pid, int* totalDllsToLoad, int* numDllsLoaded)
{
    *totalDllsToLoad = CSGODLLs.size();
    *numDllsLoaded = 0;

    bool WindowOpen = false;
    {
        HWND hCurWnd = nullptr;
        do
        {
            hCurWnd = FindWindowEx(nullptr, hCurWnd, nullptr, nullptr);
            DWORD WindowProcessID = 0;
            GetWindowThreadProcessId(hCurWnd, &WindowProcessID);
            if (WindowProcessID == pid)
            {
                WindowOpen = true;
                break;
            }
        } while (hCurWnd != nullptr);
    }
    if (!WindowOpen)
        return false;

    bool allModulesLoaded = true;
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        MODULEENTRY32 moduleEntry{};
        moduleEntry.dwSize = sizeof(moduleEntry);

        do
        {
            for (int i = 0; i < *totalDllsToLoad; i++)
            {
                if (CSGODLLs.at(i) == moduleEntry.szModule)
                    *numDllsLoaded++;
            }
        } while (Module32Next(snapshot, &moduleEntry));
        CloseHandle(snapshot);

        allModulesLoaded = *numDllsLoaded >= *totalDllsToLoad;
    }
    if (!allModulesLoaded)
        return false;

    return true;
}

bool MainFrame::HandleFindCSGOStep()
{
    Step* s = this->Steps + 1;

    bool wasAlreadyOpen = true;
    HANDLE proc;
    while (!(proc = getCSGO()))
    {
        wasAlreadyOpen = false;
        s->State = "Please open CS:GO.";
        Sleep(1000);
    }

    this->CSGO = proc;
    this->CSGO_PID = GetProcessId(proc);
    if (!this->CSGO || !this->CSGO_PID)
    {
        s->State = std::string("Failed to open CS:GO - ") + ERROR_STR((DWORD)(this->CSGO), this->CSGO_PID);
        return false;
    }

    int progress = 0;
    int total = 0;
    while (!csgoIsInitialized(this->CSGO, this->CSGO_PID, &total, &progress))
    {
        wasAlreadyOpen = false;
        s->State = "Found - Waiting for CS:GO to initialize... (" + std::to_string(progress) + "/" + std::to_string(total) + ")";
        Sleep(1000);

        DWORD ExitCode = 0;
        if (!GetExitCodeProcess(this->CSGO, &ExitCode) || ExitCode != STILL_ACTIVE)
        {
            // those fuckers closed csgo before it initialized >:(
            s->State = "FAILED - CSGO has been closed.";
            return false;
        }
    }

    if (!wasAlreadyOpen)
    {
        s->State = "Found - Loading...";
        Sleep(3000);
    }

    s->State = "Success!";
    return true;
}

bool MainFrame::HandleDownloadStep()
{
    Step* s = this->Steps + 2;

    size_t bytesRead = 0;
    s->State = "Downloading...";
    char* result = HTTP::GET("https://www.a4g4.com/API/dll/download2.php", &bytesRead);
    if (!result || bytesRead < 10000)
    {
        s->State = "FAILED - Check your firewall. " + ERROR_STR(1,1);
        return false;
    }
    this->DLLHeader = Encryption::parseHeader((uint8_t*)result, bytesRead);
    if (!this->DLLHeader.isValid)
    {
        s->State = "FAILED - " + this->DLLHeader.parseError;
        return false;
    }
    this->DLLFileSize = bytesRead;
    this->DLLFile = result;
    s->State = "Download complete.";
    return true;
}

bool MainFrame::HandleInjectStep()
{
    Step* s = this->Steps + 3;
    s->State = "Injecting...";

    uint64_t dllSize = Encryption::getDecryptedSize(this->DLLHeader, this->DLLFileSize);

    auto mapper = ManualMapper(this->CSGO);
    DWORD seekAddr = 0;
    bool doneMapping = false;
    byte* decryptedFileBuffer = (byte*)malloc(252);
    size_t nChunks = (size_t)((this->DLLFileSize - this->DLLHeader.size) / 256);
    while (!mapper.Errored() && !doneMapping)
    {
        seekAddr = mapper.GetNextFileSeekLocation();
        if (seekAddr > dllSize)
        {
            s->State = "Failed - Attempted to access past EOF.";
            return false;
        }
        s->State = "Processing chunk @ " + std::to_string(seekAddr);

        // map seekAddress to fileAddress
        size_t chunkIndex = seekAddr / 252;
        uint64_t chunkBase = (uint64_t)chunkIndex * (uint64_t)256 + (uint64_t)this->DLLHeader.size;
        Encryption::decryptChunk(this->DLLHeader, chunkIndex, (uint8_t*)this->DLLFile + chunkBase, decryptedFileBuffer);

        bool isLastChunk = (chunkIndex + 1) >= nChunks;
        if (isLastChunk)
        {
            for (int i = 0; i < 252 - this->DLLHeader.endPadding; i++)
            {
                decryptedFileBuffer[i] = decryptedFileBuffer[i + this->DLLHeader.endPadding];
            }
        }
        byte* ptrToSoughtData = decryptedFileBuffer + seekAddr % 252;
        size_t soughtDataSize = (isLastChunk ? 252 - this->DLLHeader.endPadding : 252) - seekAddr % 252;

        doneMapping = mapper.ProcessBytesFromFile(ptrToSoughtData, soughtDataSize);
    }
    if (!mapper.Errored())
    {
        s->State = "Executing entry point...";
        mapper.Execute();
    }
    else
    {
        s->State = "FAILED - " + ERROR_STR(mapper.GetErrorContext(), mapper.GetErrorCode());
        return false;
    }
    free(this->DLLFile);
    s->State = "Success!";
    return true;
}

void MainFrame::Exit()
{
    this->repainter->Delete();
    this->Close(true);
}

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_LEFT_DOWN(MainFrame::OnMouseDown)
EVT_LEFT_UP(MainFrame::OnMouseUp)
EVT_PAINT(MainFrame::OnPaint)
EVT_MOTION(MainFrame::OnMouseMove)
EVT_LEAVE_WINDOW(MainFrame::OnLeaveWindow)
EVT_ERASE_BACKGROUND(MainFrame::OnEraseBackground)
wxEND_EVENT_TABLE()