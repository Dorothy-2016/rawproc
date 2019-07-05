#include "PicProcessorTone.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myToneCurvePane.h"
#include "myFloatCtrl.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"
#include "copy.xpm"
#include "paste.xpm"

#define TONEENABLE 7900
#define TONEID 7901
#define TONECOPY 7902
#define TONEPASTE 7903
#define TONEGAMMA 7904
#define TONEREINHARD 7905
#define TONELOG2 7906
#define TONELOGGAM 7907
#define TONEFILMIC 7908
#define TONECURVE 7909
#define TONENORM 7910

class ToneCurveDialog: public wxDialog
{
	public:
		ToneCurveDialog(wxWindow *parent, wxWindowID id, const wxString &title, std::vector<float> xarray, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize):
		wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
		{
			tcpane = new myToneCurvePane(this, wxDefaultPosition, wxSize(110,110));
			wxBoxSizer* s = new wxBoxSizer( wxVERTICAL );
			s->Add(tcpane, 1, wxEXPAND, 5);
			SetSizerAndFit(s);
			s->Layout();
			tcpane->SetCurve(xarray, true);
		}

		~ToneCurveDialog() {}

		void SetCurve(std::vector<float> curve)
		{
			tcpane->SetCurve(curve);
			Refresh();
		}

	private:
		myToneCurvePane *tcpane;
};

class TonePanel: public PicProcPanel
{

	public:
		TonePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);


			enablebox = new wxCheckBox(this, TONEENABLE, "tone:");
			enablebox->SetValue(true);

			//All the radio buttons in the same group:
			gamb = new wxRadioButton(this, TONEGAMMA, "gamma", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			reinb = new wxRadioButton(this, TONEREINHARD, "reinhard");
			//log2b = new wxRadioButton(this, TONELOG2, "log2");
			hybloggamb = new wxRadioButton(this, TONELOGGAM, "loggamma");
			filmicb = new wxRadioButton(this, TONEFILMIC, "filmic");

			tonenorm = new wxCheckBox(this, TONENORM, "norm:");
			tonenorm->SetValue(false);

			tcpane = new myToneCurvePane(this, wxDefaultPosition, wxSize(130,130));
			//tc = NULL;

			//parm tool.tone.gamma: Default value for gamma tone operator.  Default=2.2
			gamma = new myFloatCtrl(this, wxID_ANY, atof(myConfig::getConfig().getValueOrDefault("tool.tone.gamma","2.2").c_str()), 2);

			//parm tool.tone.filmic.A: Default value for filmic tone operator A coefficient.  Default=6.2
			filmicA = new myFloatCtrl(this, wxID_ANY, "A:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.A","6.2").c_str()), 1, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			//parm tool.tone.filmic.B: Default value for filmic tone operator B coefficient.  Default=0.5
			filmicB = new myFloatCtrl(this, wxID_ANY, "B:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.B","0.5").c_str()), 2, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			//parm tool.tone.filmic.C: Default value for filmic tone operator C coefficient.  Default=1.7
			filmicC = new myFloatCtrl(this, wxID_ANY, "C:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.C","1.7").c_str()), 1, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			//parm tool.tone.filmic.D: Default value for filmic tone operator D coefficient.  Default=0.06
			filmicD = new myFloatCtrl(this, wxID_ANY, "D:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.D","0.06").c_str()), 2, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			//parm tool.tone.filmic.power: Default value for filmic tone operator power coefficient.  Set this to 1.0 to remove the effect of this coefficient.  Default=1.0
			power   = new myFloatCtrl(this, wxID_ANY, "power:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.power","1.0").c_str()), 1);

			wxArrayString str;
			str.Add("channel");
			str.Add("luminance");
			reinop = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(85,TEXTCTRLHEIGHT), str);

			wxArrayString p = split(params,",");
			setPanel(p);

			tcpane->SetCurve(makeXArray(), true);

			//log2b->Enable(false);  //log2 doesn't do anything, yet.

			//Lay out the controls in the panel:
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, TONECOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, TONEPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(gamb, flags);
			m->AddRowItem(gamma, flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(reinb, flags);
			m->AddRowItem(reinop, flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			//m->AddRowItem(log2b, flags);
			//m->NextRow();
			m->AddRowItem(hybloggamb, flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(filmicb, flags);
			m->AddRowItem(power, flags);
			m->NextRow(); 
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "     "), flags);
			m->AddRowItem(filmicA, flags);
			//m->NextRow();
			m->AddRowItem(filmicB, flags);
			m->NextRow(); 
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "     "), flags);
			m->AddRowItem(filmicC, flags);
			//m->NextRow();
			m->AddRowItem(filmicD, flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "tone curve:"), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "  "), wxSizerFlags(1).Left());
			m->AddRowItem(tcpane, wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP));
			m->AddRowItem(tonenorm, flags);
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "  "), wxSizerFlags(1).Left());
			m->End();
			SetSizerAndFit(m);
			m->Layout();

			SetFocus();
			t = new wxTimer(this);

			Bind(wxEVT_TIMER, &TonePanel::OnTimer, this);
			Bind(wxEVT_BUTTON, &TonePanel::OnCopy, this, TONECOPY);
			Bind(wxEVT_BUTTON, &TonePanel::OnPaste, this, TONEPASTE);
			Bind(myFLOATCTRL_CHANGE, &TonePanel::floatParamChanged, this);
			Bind(myFLOATCTRL_UPDATE, &TonePanel::floatParamUpdated, this);
			Bind(wxEVT_CHECKBOX, &TonePanel::onEnable, this, TONEENABLE);
			Bind(wxEVT_CHECKBOX, &TonePanel::OnNorm, this, TONENORM);
			Bind(wxEVT_RADIOBUTTON, &TonePanel::OnButton, this);
			Bind(wxEVT_CHOICE, &TonePanel::reinopChanged, this);
			Refresh();
			Update();
		}

		~TonePanel()
		{
			t->~wxTimer();
		}

		void setPanel(wxArrayString p)
		{
			if (p[0] == "gamma") {
				gamb->SetValue(true);
				tonemode = TONEGAMMA;
				if (p.GetCount() >=2) 
					gamma->SetFloatValue(atof(p[1].c_str()));
				else
					gamma->SetFloatValue(1.0);
			}
			else {
				gamma->SetFloatValue(1.0);
			}
			if (p[0] == "reinhard") {
				reinb->SetValue(true);
				tonemode = TONEREINHARD;
				if (p.GetCount() >=2) 
					reinop->SetStringSelection(p[1]);
				else
					reinop->SetSelection(reinop->FindString("channel"));
				if (p.GetCount() >=3) 
					if (p[2] == "norm")
						tonenorm->SetValue(true);
			}
			else {
				reinop->SetSelection(reinop->FindString("channel"));
			}
			//if (p[0] == "log2") {
			//	log2b->SetValue(true);
			//}
			if (p[0] == "loggamma") {
				hybloggamb->SetValue(true);
				tonemode = TONELOGGAM;
			}
			if (p[0] == "filmic") {
				filmicb->SetValue(true);
				tonemode = TONEFILMIC;
				if (p.GetCount() >=6) {
					filmicA->SetFloatValue(atof(p[1].c_str()));
					filmicB->SetFloatValue(atof(p[2].c_str()));
					filmicC->SetFloatValue(atof(p[3].c_str()));
					filmicD->SetFloatValue(atof(p[4].c_str()));
					power->SetFloatValue(atof(p[5].c_str()));
				}
				if (p.GetCount() >=7) 
					if (p[6] == "norm")
						tonenorm->SetValue(true);
			}

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

		void OnCopy(wxCommandEvent& event)
		{
			q->copyParamsToClipboard();
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format("Copied command to clipboard: %s",q->getCommand()));
			
		}

		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {
				wxArrayString p = split(q->getParams(),",");
				setPanel(p);
				processTone(event.GetId());
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format("Pasted command from clipboard: %s",q->getCommand()));
			}
			else wxMessageBox(wxString::Format("Invalid Paste"));
		}

		void reinopChanged(wxCommandEvent& event)
		{
			if (reinb->GetValue()) {
				if (tonenorm->GetValue())
					q->setParams(wxString::Format("reinhard,%s,norm",reinop->GetString(reinop->GetSelection())));
				else
					q->setParams(wxString::Format("reinhard,%s",reinop->GetString(reinop->GetSelection())));
				q->processPic();
			}
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			processTone(event.GetId());
			tonemode = event.GetId();
			event.Skip();
		}

		void OnNorm(wxCommandEvent& event)
		{
			processTone(tonemode);
			event.Skip();
		}

		void processTone(int src)
		{
			switch (src) {
				case TONEGAMMA:
					q->setParams(wxString::Format("gamma,%0.2f",gamma->GetFloatValue()));
					break;
				case TONEREINHARD:
					if (tonenorm->GetValue())
						q->setParams(wxString::Format("reinhard,%s,norm",reinop->GetString(reinop->GetSelection())));
					else
						q->setParams(wxString::Format("reinhard,%s",reinop->GetString(reinop->GetSelection())));
					break;
				case TONELOG2:
					q->setParams(wxString::Format("log2"));
					break;
				case TONELOGGAM:
					q->setParams(wxString::Format("loggamma"));
					break;
				case TONEFILMIC:
					if (tonenorm->GetValue())
						q->setParams(wxString::Format("filmic,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,norm",filmicA->GetFloatValue(),filmicB->GetFloatValue(),filmicC->GetFloatValue(),filmicD->GetFloatValue(),power->GetFloatValue()));
					else
						q->setParams(wxString::Format("filmic,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f",filmicA->GetFloatValue(),filmicB->GetFloatValue(),filmicC->GetFloatValue(),filmicD->GetFloatValue(),power->GetFloatValue()));
					break;
			}
			//if (tc) tc->SetCurve(makeXArray());
			tcpane->SetCurve(makeXArray());
			q->processPic();
			Refresh();
		}


		void floatParamChanged(wxCommandEvent& event)
		{
			if (gamb->GetValue() | filmicb->GetValue()) t->Start(500,wxTIMER_ONE_SHOT);
		}
		
		void floatParamUpdated(wxCommandEvent& event)
		{
			if (gamb->GetValue()) processTone(TONEGAMMA);
			if (filmicb->GetValue()) processTone(TONEFILMIC);
		}
		
		void OnTimer(wxTimerEvent& event)
		{
			if (gamb->GetValue()) processTone(TONEGAMMA);
			if (filmicb->GetValue()) processTone(TONEFILMIC);
		}

		std::vector<float> makeXArray()
		{
			std::vector<float> xarray;
			std::map<std::string,std::string> imgdata;
			gImage X(10000,1,3,imgdata);
			std::vector<pix>& x = X.getImageData();
			for (unsigned i=0; i<x.size(); i++) x[i].r = (float) i / x.size();

			//tone map operation on X, builds transform curve for plotting
			if (gamb->GetValue()) X.ApplyToneMapGamma(gamma->GetFloatValue());
			else if (reinb->GetValue() ) {
				bool channel = true;
				if (reinop->GetString(reinop->GetSelection()) == "luminance") channel = false;
				X.ApplyToneMapReinhard(channel, tonenorm->GetValue());
			}
			else if (hybloggamb->GetValue()) X.ApplyToneMapLogGamma();
			else if (filmicb->GetValue())  X.ApplyToneMapFilmic(filmicA->GetFloatValue(),filmicB->GetFloatValue(),filmicC->GetFloatValue(),filmicD->GetFloatValue(),power->GetFloatValue(), tonenorm->GetValue());

			for (unsigned i=0; i<x.size(); i++) xarray.push_back(x[i].r);
			return xarray;
		}

	private:
		wxTimer *t;
		myFloatCtrl *gamma, *filmicA, *filmicB, *filmicC, *filmicD, *power;
		wxCheckBox *enablebox, *tonenorm;
		wxRadioButton *gamb, *reinb, *log2b, *hybloggamb, *filmicb;
		wxChoice *reinop;
		myToneCurvePane *tcpane;
		int tonemode;
		//ToneCurveDialog *tc;
};

PicProcessorTone::PicProcessorTone(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorTone::createPanel(wxSimplebook* parent)
{
	toolpanel = new TonePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorTone::processPic(bool processnext) 
{
	wxString d;
	wxArrayString p = split(c,",");
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.tone.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (!global_processing_enabled) return true;


	if (processingenabled) {
		if (p[0] == "gamma") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: gamma...");
			m_tree->SetItemText(id, "tone:gamma");
			double gamma = 1.0;
			if (p.size() >= 2) gamma = atof(p[1].c_str());
			mark();
			dib->ApplyToneMapGamma(gamma, threadcount);
			d = duration();

		}
		else if (p[0] == "loggamma") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: hybrid log gamma...");
			m_tree->SetItemText(id, "tone:loggamma");
			mark();
			dib->ApplyToneMapLogGamma(threadcount);
			d = duration();

		}
		else if (p[0] == "log2") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: log2...");
			m_tree->SetItemText(id, "tone:log2");
			mark();
			dib->ApplyToneMapLog2(threadcount);
			d = duration();
		}
		else if (p[0] == "reinhard") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: reinhard...");
			m_tree->SetItemText(id, "tone:reinhard");
			bool channel = true;
			if (p.size() >= 2) if (p[1] == "luminance") channel = false;
			bool norm = false;
			if (p.size() >= 3) if (p[2] == "norm") norm = true;
			mark();
			dib->ApplyToneMapReinhard(channel, norm, threadcount);
			d = duration();
		}
		else if (p[0] == "filmic") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: filmic...");
			m_tree->SetItemText(id, "tone:filmic");
			double filmicA = 6.2;
			double filmicB = 0.5;
			double filmicC = 1.7;
			double filmicD = 0.06;
			double power = 2.2;
			bool norm = false;
			if (p.size() >= 2) filmicA = atof(p[1].c_str());
			if (p.size() >= 3) filmicB = atof(p[2].c_str());
			if (p.size() >= 4) filmicC = atof(p[3].c_str());
			if (p.size() >= 5) filmicD = atof(p[4].c_str());
			if (p.size() >= 6) power = atof(p[5].c_str());
			if (p.size() >= 7 && p[6] == "norm") norm = true;
			mark();
			dib->ApplyToneMapFilmic(filmicA, filmicB, filmicC, filmicD, power, norm, threadcount);
			d = duration();
		}

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.tone.log","0") == "1"))
			log(wxString::Format("tool=tone:%s,imagesize=%dx%d,threads=%d,time=%s",p[0],dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





