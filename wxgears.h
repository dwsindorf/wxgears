/////////////////////////////////////////////////////////////////////////////
// Name:        wxtest.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TEST_H_
#define _WX_TEST_H_

#include "wx/defs.h"
#include "wx/app.h"
#include "wx/menu.h"
#include "wx/dcclient.h"

#include "wx/glcanvas.h"

/* Define a new application type */
class MyApp: public wxApp
{
public:
    bool OnInit();
};

/* Define a new frame type */
class TestGLCanvas;

class MyFrame: public wxFrame
{
    wxTimer m_timer;
    bool running;
    wxCheckBox *m_gradient;
    wxButton *m_start;
	wxTextCtrl *m_frame_rate;
	wxBoxSizer *m_topSizer;
public:
    MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

    void OnSize(wxSizeEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnStart(wxCommandEvent& event);
    void OnShowGradient(wxCommandEvent& event);
    void OnUpdateShowGradient(wxUpdateUIEvent& event);

    void startTimer();
//    void SetCanvas( TestGLCanvas *canvas ) { m_canvas = canvas; }
//    TestGLCanvas *GetCanvas() { return m_canvas; }
private:
    TestGLCanvas *m_canvas;
    DECLARE_EVENT_TABLE()
};


class TestGLCanvas: public wxGLCanvas
{
	double view_rotx, view_roty, view_rotz,angle;
	GLint gear1, gear2, gear3;
	GLubyte		*pixels;

	wxStopWatch stopwatch;
	int dcnt,fcnt;
    int height;
    int width;
    int prevMouseX,prevMouseY;
    void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
    		GLint teeth, GLfloat tooth_depth);
    void InitGL();
    void init_gears();
    void draw_gears();
    void draw_gradient();
    void make_gradient();
    void render();
public:
    int timer_interval;

	double frame_rate;
	double elaped_time;
	bool running;
	bool show_gradient;

    TestGLCanvas(wxWindow *parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxString& name = wxT("TestGLCanvas"));

    void time_tick();
    void OnSize(wxSizeEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnMouseDown( wxMouseEvent& event );
    void OnMouseMove( wxMouseEvent& event);
    void animate();
    void reset();
    void reshape(int width, int height);


private:
    DECLARE_EVENT_TABLE()
};

#endif //

