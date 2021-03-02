#include "DemoApp.h"
#include "DemoFrame.h"

IMPLEMENT_APP(DemoApp);

bool DemoApp::OnInit()
{
    DemoFrame* frame = new DemoFrame(nullptr, _("wxOpenCV Demo"));
    frame->Show();

    return true;
}
