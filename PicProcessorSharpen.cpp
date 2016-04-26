#include "PicProcessor.h"
#include "PicProcessorSharpen.h"
#include "ThreadedConvolve.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "myTouchSlider.h"
#include "util.h"

#include <vector>
#include <wx/fileconf.h>

class SharpenPanel: public PicProcPanel
{
	public:
		SharpenPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			b->SetOrientation(wxHORIZONTAL);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			slide = new myTouchSlider((wxFrame *) this, wxID_ANY, "sharpen", SLIDERWIDTH, atof(p.c_str()), 1.0, 0.0, 10.0, "%2.0f");
			b->Add(100,100,1);
			b->Add(slide, flags);
			b->Add(100,100,1);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(SharpenPanel::paramChanged));
		}

		~SharpenPanel()
		{
			slide->~myTouchSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%d",slide->GetIntValue()));
			q->processPic();
			event.Skip();
		}


	private:
		myTouchSlider *slide;

};


PicProcessorSharpen::PicProcessorSharpen(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorSharpen::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new SharpenPanel(m_parameters, this, c);
}


bool PicProcessorSharpen::processPic() {
	double kernel[3][3] =
	{
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	};

	std::vector<ThreadedConvolve *> t;
	int threadcount;

	m_tree->SetItemBold(GetId(), true);

	double sharp = atof(c.c_str())+1.0;
	double x = -((sharp-1)/4.0);
	kernel[0][1] = x;
	kernel[1][0] = x;
	kernel[1][2] = x;
	kernel[2][1] = x;
	kernel[1][1] = sharp;
	bool result = true;

	wxConfigBase::Get()->Read("tool.sharpen.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) wxThread::GetCPUCount();
	((wxFrame*) m_parameters->GetParent())->SetStatusText(wxString::Format("sharpen, %d cores...",threadcount));
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());

	if (sharp > 1.0) {
		mark();
		for (int i=0; i<threadcount; i++) {
			t.push_back(new ThreadedConvolve(getPreviousPicProcessor()->getProcessedPic(), dib, i,threadcount, kernel));
			t.back()->Run();
		}
		while (!t.empty()) {
			t.back()->Wait(wxTHREAD_WAIT_BLOCK);
			t.pop_back();
		}
		wxString d = duration();
		if (wxConfigBase::Get()->Read("tool.sharpen.log","0") == "1")
			log(wxString::Format("tool=sharpen,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

	}
	dirty = false;

	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	m_tree->SetItemBold(GetId(), false);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}
	m_tree->SetItemBold(GetId(), false);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");

	return result;
}



