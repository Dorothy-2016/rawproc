#include "PicProcessor.h"
#include "PicProcessorDenoise.h"
#include "PicProcPanel.h"
#include "myFloatCtrl.h"
#include "myConfig.h"
#include "undo.xpm"

#include "util.h"

#define DENOISEENABLE 6000

#define DENOISENLMEANS 6001
#define DENOISEWAVELET 6002

#define SIGMASLIDER 6005
#define LOCALSLIDER 6006
#define PATCHSLIDER 6007

#define WAVELETTHRESHOLD 6008

#define SIGMARESET 6010
#define LOCALRESET 6011
#define PATCHRESET 6012

class DenoisePanel: public PicProcPanel
{
	public:
		DenoisePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			algorithm = DENOISENLMEANS;
			int sigmaval = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.initialvalue","0").c_str());
			int localval = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.local","3").c_str());
			int patchval = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.patch","1").c_str());
			float thresholdval = atof(myConfig::getConfig().getValueOrDefault("tool.denoise.threshold","0.0").c_str());
			
			wxSize spinsize(130, TEXTCTRLHEIGHT);
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			enablebox = new wxCheckBox(this, DENOISEENABLE, "denoise:");
			enablebox->SetValue(true);
			g->Add(enablebox, wxGBPosition(0,0), wxGBSpan(1,2), wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(200,2)),  wxGBPosition(1,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);


			nl = new wxRadioButton(this, DENOISENLMEANS, "NLMeans:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			nl->SetValue(true);
			g->Add(nl,  wxGBPosition(2,0), wxGBSpan(1,2), wxALIGN_LEFT | wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "sigma: "), wxGBPosition(3,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			sigma = new wxSlider(this, SIGMASLIDER, sigmaval, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(sigma , wxGBPosition(3,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val = new wxStaticText(this,wxID_ANY, wxString::Format("%d",sigmaval), wxDefaultPosition, wxSize(30, -1));
			g->Add(val , wxGBPosition(3,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn = new wxBitmapButton(this, SIGMARESET, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset to default");
			g->Add(btn, wxGBPosition(3,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			g->Add(0,20, wxGBPosition(4,0));

			g->Add(new wxStaticText(this,wxID_ANY, "local: "), wxGBPosition(5,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			local = new wxSlider(this, LOCALSLIDER, localval, 0, 15, wxPoint(10, 30), wxSize(140, -1));
			g->Add(local , wxGBPosition(5,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val1 = new wxStaticText(this,wxID_ANY, wxString::Format("%d",localval), wxDefaultPosition, wxSize(30, -1));
			g->Add(val1 , wxGBPosition(5,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn1 = new wxBitmapButton(this, LOCALRESET, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn1->SetToolTip("Reset to default");
			g->Add(btn1, wxGBPosition(5,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "patch: "), wxGBPosition(6,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			patch = new wxSlider(this, PATCHSLIDER, patchval, 0, 15, wxPoint(10, 30), wxSize(140, -1));
			g->Add(patch , wxGBPosition(6,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val2 = new wxStaticText(this,wxID_ANY, wxString::Format("%d",patchval), wxDefaultPosition, wxSize(30, -1));
			g->Add(val2 , wxGBPosition(6,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn2 = new wxBitmapButton(this, PATCHRESET, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn2->SetToolTip("Reset to default");
			g->Add(btn2, wxGBPosition(6,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);


			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(200,2)),  wxGBPosition(7,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);

			wl = new wxRadioButton(this, DENOISEWAVELET, "Wavelet:");
			g->Add(wl,  wxGBPosition(8,0), wxGBSpan(1,2), wxALIGN_LEFT | wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "threshold:"), wxGBPosition(9,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 2);
			thresh = new myFloatCtrl(this, WAVELETTHRESHOLD, thresholdval, 4, wxDefaultPosition, spinsize);
			g->Add(thresh, wxGBPosition(9,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 2);
			
			bool nlb, wlb;
			wxArrayString cp = split(params,",");
			if (cp.GetCount() == 4 && cp[0] == "nlmeans") {
				sigma->SetValue(atoi(cp[1])); val->SetLabel(wxString::Format("%4d", sigma->GetValue()));
				local->SetValue(atoi(cp[2])); val1->SetLabel(wxString::Format("%4d", local->GetValue()));
				patch->SetValue(atoi(cp[3])); val2->SetLabel(wxString::Format("%4d", patch->GetValue()));
				nlb=true; wlb=false;
				algorithm = DENOISENLMEANS;
				nl->SetValue(true);
			}
			if (cp.GetCount() == 2 && cp[0] == "wavelet") {
				thresh->SetFloatValue(atof(cp[1]));
				nlb=false; wlb=true;
				algorithm = DENOISEWAVELET;
				wl->SetValue(true);
			}

			sigma->Enable(nlb); 
			local->Enable(nlb); 
			patch->Enable(nlb);
			val->Enable(nlb);
			val1->Enable(nlb); 
			val2->Enable(nlb);
			btn->Enable(nlb);
			btn1->Enable(nlb);
			btn2->Enable(nlb);
			thresh->Enable(wlb);

			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &DenoisePanel::OnButton, this);
			Bind(wxEVT_RADIOBUTTON, &DenoisePanel::onRadioButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &DenoisePanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &DenoisePanel::OnThumbTrack, this);
			Bind(wxEVT_TIMER, &DenoisePanel::OnTimer,  this);
			Bind(wxEVT_CHECKBOX, &DenoisePanel::onEnable, this, DENOISEENABLE);
			Bind(wxEVT_MOUSEWHEEL,&DenoisePanel::onWheel, this);
			//Bind(wxEVT_TEXT_ENTER, &DenoisePanel::paramChanged, this);
		}

		~DenoisePanel()
		{
			t->~wxTimer();
		}

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				q->enableProcessing(true);
				q->processPic();
			}
			else {
				q->enableProcessing(false);
				q->processPic();
			}
		}

		void onRadioButton(wxCommandEvent& event)
		{
			if (event.GetId() == DENOISENLMEANS) {
				sigma->Enable(true); 
				local->Enable(true); 
				patch->Enable(true);
				val->Enable(true);
				val1->Enable(true); 
				val2->Enable(true);
				btn->Enable(true);
				btn1->Enable(true);
				btn2->Enable(true);

				thresh->Enable(false);
				algorithm = DENOISENLMEANS;
			}
			if (event.GetId() == DENOISEWAVELET) {
				thresh->Enable(true);
				sigma->Enable(false); 
				local->Enable(false); 
				patch->Enable(false);
				val->Enable(false);
				val1->Enable(false); 
				val2->Enable(false);
				btn->Enable(false);
				btn1->Enable(false);
				btn2->Enable(false);
				algorithm = DENOISEWAVELET;
			}
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void onWheel(wxMouseEvent& event)
		{
			//if (event.GetId() == WAVELETTHRESHOLD) {
				if (thresh->GetFloatValue() < 0.0) thresh->SetFloatValue(0.0);
				if (thresh->GetFloatValue() > 1.0) thresh->SetFloatValue(1.0);
				t->Start(500,wxTIMER_ONE_SHOT);
			//}
			event.Skip();
		}

		void OnChanged(wxCommandEvent& event)
		{
			if (event.GetId() == SIGMASLIDER) val->SetLabel(wxString::Format("%4d", sigma->GetValue()));
			if (event.GetId() == LOCALSLIDER) val1->SetLabel(wxString::Format("%4d", local->GetValue()));
			if (event.GetId() == PATCHSLIDER) val2->SetLabel(wxString::Format("%4d", patch->GetValue()));
			if (event.GetId() == SIGMASLIDER) t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			if (event.GetId() == SIGMASLIDER) val->SetLabel(wxString::Format("%4d", sigma->GetValue()));
			if (event.GetId() == LOCALSLIDER) val1->SetLabel(wxString::Format("%4d", local->GetValue()));
			if (event.GetId() == PATCHSLIDER) val2->SetLabel(wxString::Format("%4d", patch->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			if (algorithm == DENOISENLMEANS)
				q->setParams(wxString::Format("nlmeans,%d,%d,%d",sigma->GetValue(),local->GetValue(),patch->GetValue()));
			if (algorithm == DENOISEWAVELET) 
				q->setParams(wxString::Format("wavelet,%f",thresh->GetFloatValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			int sigmareset, localreset, patchreset;
			if (event.GetId() == SIGMARESET) {
				sigmareset = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.initialvalue","0").c_str());
				sigma->SetValue(sigmareset);
				val->SetLabel(wxString::Format("%4d", sigmareset));
				q->setParams(wxString::Format("%d,%d,%d",sigma->GetValue(),local->GetValue(),patch->GetValue()));
				q->processPic();
			}
			if (event.GetId() == LOCALRESET) {
				localreset = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.local","3").c_str());
				local->SetValue(localreset);
				val1->SetLabel(wxString::Format("%4d", localreset));
			}
			if (event.GetId() == PATCHRESET) {
				patchreset = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.patch","1").c_str());
				patch->SetValue(patchreset);
				val2->SetLabel(wxString::Format("%4d", patchreset));
			}

			event.Skip();
		}


	private:
		int algorithm;
		wxRadioButton *nl, *wl;
		wxSlider *sigma, *local, *patch;
		myFloatCtrl *thresh;
		wxStaticText *val, *val1, *val2;
		wxBitmapButton *btn, *btn1, *btn2;
		wxCheckBox *enablebox;
		wxTimer *t;

};


PicProcessorDenoise::PicProcessorDenoise(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorDenoise::createPanel(wxSimplebook* parent)
{
	toolpanel = new DenoisePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorDenoise::processPic(bool processnext) {
	((wxFrame*) m_display->GetParent())->SetStatusText("denoise...");

	int algorithm = DENOISENLMEANS;
	double sigma = 0.0;
	int local = 3;
	int patch = 1;

	double threshold = 0.0001;

	wxArrayString cp = split(getParams(),",");
	if (cp.GetCount() == 4 && cp[0] == "nlmeans") {
		sigma = atof(cp[1]);
		local = atoi(cp[2]);
		patch = atoi(cp[3]);
		algorithm = DENOISENLMEANS;
	}
	if (cp.GetCount() == 2 && cp[0] == "wavelet") {
		threshold = atof(cp[1]);
		algorithm = DENOISEWAVELET;
	}
	else if (cp.GetCount() == 3) { //legacy
		sigma = atof(cp[0]);
		local = atoi(cp[1]);
		patch = atoi(cp[2]);
		algorithm = DENOISENLMEANS;
	}
	

	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (processingenabled) { 
		mark();
		if (algorithm == DENOISENLMEANS)
			if (sigma > 0.0) dib->ApplyNLMeans(sigma,local, patch, threadcount);
		if (algorithm == DENOISEWAVELET)
			if (threshold > 0.0) dib->ApplyWaveletDenoise(threshold, threadcount);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.denoise.log","0") == "1"))
			if (algorithm == DENOISENLMEANS)
				log(wxString::Format("tool=denoise_nlmeans,sigma=%2.2f,local=%d,patch=%d,imagesize=%dx%d,threads=%d,time=%s",sigma,local,patch,dib->getWidth(),dib->getHeight(),threadcount,d));
			if (algorithm == DENOISEWAVELET)
				log(wxString::Format("tool=denoise_wavelet,threshold=%f, imagesize=%dx%d,threads=%d,time=%s",threshold,dib->getWidth(),dib->getHeight(),threadcount,d));
	}

	dirty=false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();

	return result;
}



