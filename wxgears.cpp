/////////////////////////////////////////////////////////////////////////////
// Name:        TestClass.cpp
/////////////////////////////////////////////////////////////////////////////

//#define __GXX_ABI_VERSION 1002

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif


#include "wxgears.h"
#ifdef __DARWIN__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif


#include "sample.xpm"
#include <iostream>

using namespace std;
//#define DEBUG_TIMER
//#define DEBUG_EVENTS
#define TIMER_ID 1

static int tcount=0;

//************************************************************
// MyApp class
//************************************************************
// `Main program' equivalent, creating windows and returning main app frame
bool MyApp::OnInit()
{
	//wxSize init_size(1200,900);
	wxSize init_size(600,400);
    // Create the main frame window
    MyFrame *frame = new MyFrame(NULL, wxT("OpenGL Performance Test 2"),
        wxDefaultPosition, init_size);
    return true;
}

IMPLEMENT_APP(MyApp)


//************************************************************
// MyFrame class
//************************************************************
enum{
	ID_SHOW_GRADIENT,
	ID_START,
	ID_FRAMERATE,
};
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	//EVT_SIZE(MyFrame::OnSize)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
    EVT_TIMER(TIMER_ID, MyFrame::OnTimer)
    EVT_BUTTON(ID_START,MyFrame::OnStart)
    EVT_CHECKBOX(ID_SHOW_GRADIENT,MyFrame::OnShowGradient)
    EVT_UPDATE_UI(ID_SHOW_GRADIENT, MyFrame::OnUpdateShowGradient)
END_EVENT_TABLE()

/* My frame constructor */
MyFrame::MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
    const wxSize& size, long style)
    : wxFrame(frame, wxID_ANY, title, pos, size, style) , m_timer(this, TIMER_ID)
{
	m_topSizer = new wxBoxSizer(wxVERTICAL);

 	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

 	m_frame_rate=new wxTextCtrl(this,ID_FRAMERATE,wxT(""),wxDefaultPosition,wxSize(100,25),wxTE_READONLY);
    m_gradient=new wxCheckBox(this,ID_SHOW_GRADIENT,wxT("Gradient"),wxDefaultPosition,wxSize(100,25));
    m_start=new wxButton(this,ID_START,wxT("Start"),wxDefaultPosition,wxSize(100,25));

    wxStaticText *label=new wxStaticText(this,-1,wxT("FPS:"),wxDefaultPosition,wxSize(40,25));
    hline->Add(label,0,wxALIGN_LEFT|wxALL,5);
 	hline->Add(m_frame_rate,0,wxALIGN_LEFT|wxALL,5);
 	hline->Add(m_gradient,0,wxALIGN_LEFT|wxALL,5);
 	hline->Add(m_start,1,wxALIGN_RIGHT|wxALL,5);
#ifdef __WXMSW__
 	hline->SetMinSize(wxSize(-1,70));
#else
 	hline->SetMinSize(wxSize(-1,40));
#endif
    SetIcon(wxIcon(sample_xpm));
    m_canvas=new TestGLCanvas(this, wxID_ANY, wxDefaultPosition, size, wxSUNKEN_BORDER);
    m_topSizer->Add(m_canvas,1,wxALIGN_TOP|wxEXPAND,0);
    m_topSizer->Add(hline,0,wxALIGN_BOTTOM|wxALL,0);
    SetSizerAndFit(m_topSizer);

    Centre();
    Show(true);
    running=false;
//	const GLubyte *version=glGetString(GL_VERSION);
//	cout << "OpenGL Version:"<<version <<endl;

}

void MyFrame::OnSize(wxSizeEvent& event)
{
	wxSize size=event.GetSize();
	cout << "resize: " << size.GetWidth() << " "<< size.GetHeight()<< endl;
	m_topSizer->SetDimension(0,0,size.GetWidth(),size.GetHeight());
}
/* Intercept menu commands */
void MyFrame::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnStart(wxCommandEvent& event){
	if(m_timer.IsRunning()){
	cout << "stop timer" << endl;
		running=false;
		m_timer.Stop();
		m_canvas->running=false;
	}
	else{
	cout << "start timer" << endl;
		running=true;
		m_canvas->reset();
		m_canvas->running=true;
		startTimer();
	}
}

void MyFrame::OnShowGradient(wxCommandEvent& event){
	m_canvas->show_gradient=!m_canvas->show_gradient;
	m_canvas->reset();
	m_canvas->Refresh();
}
void MyFrame::OnUpdateShowGradient(wxUpdateUIEvent& event) {
	event.Check(m_canvas->show_gradient);
}

void MyFrame::startTimer()
{
    m_timer.Start(m_canvas->timer_interval);    // 0.1 second interval
}
void MyFrame::OnTimer(wxTimerEvent& event)
{
#ifdef DEBUG_TIMER
   cout << "GLWidget::timerEvent() #" << ++tcount << endl;
#endif
    if(running && !m_canvas->running)
    	m_canvas->running=true;
    m_canvas->time_tick();
	wxString str("");
	str+=wxString::Format("%g", m_canvas->frame_rate);
	m_frame_rate->SetValue(str);
    if(running && !m_canvas->running){
    	m_canvas->running=true;
    }
    m_canvas->Refresh();
}

//************************************************************
// TestGLCanvas class
//************************************************************
static int attributeList[] = {
	WX_GL_RGBA,
    WX_GL_DOUBLEBUFFER,
    WX_GL_DEPTH_SIZE, 24,
    0
};

#ifndef M_PI
#define M_PI 3.14159265
#endif

BEGIN_EVENT_TABLE(TestGLCanvas, wxGLCanvas)
    EVT_SIZE(TestGLCanvas::OnSize)
    EVT_PAINT(TestGLCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(TestGLCanvas::OnEraseBackground)
	EVT_LEFT_DOWN(TestGLCanvas::OnMouseDown)
	EVT_MOTION(TestGLCanvas::OnMouseMove)
END_EVENT_TABLE()

TestGLCanvas::TestGLCanvas(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, id, pos, wxSize(600,400), style|wxFULL_REPAINT_ON_RESIZE, name,attributeList)
{
    angle=0;
    frame_rate=1000;
    elaped_time=0;
    fcnt=dcnt=0;
    pixels=0;
    running=false;
    show_gradient=true;
    prevMouseX=prevMouseY=0;
#ifdef __WXMSW__
    timer_interval=1; // fastest possible
#else
    timer_interval=10; // 5 per s
#endif

}

void TestGLCanvas::InitGL()
{
	static bool firstcall=true;
#ifdef DEBUG_EVENTS
    cout << "TestGLCanvas::InitGL()" <<endl;
#endif
    glClearColor(0.5,0.5,0.5,1);
    glDrawBuffer(GL_BACK);
    int w, h;
    GetClientSize(&w, &h);
    reshape(w,h);
    init_gears();
    if(firstcall){
		const GLubyte *version=glGetString(GL_VERSION);
		cout << "OpenGL Version:"<<version <<endl;
		firstcall=false;
    }
    stopwatch.Start();
}

void TestGLCanvas::reshape(int w, int h) {
	width = w;
	height = h;

	GLfloat aspect = (GLfloat) height / (GLfloat) width;

	glViewport(0, 0, (GLint) width, (GLint) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -aspect, aspect, 5.0, 60.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -40.0);

	make_gradient();

	reset();
}

void TestGLCanvas::reset() {
	elaped_time = 0;
	dcnt = fcnt = 0;
	frame_rate = 1000;

}
//-------------------------------------------------------------
// TestGLCanvas::animate() rotate cube
//-------------------------------------------------------------
void TestGLCanvas::animate()
{
#ifndef __WXMSW__
    Refresh(); // force a repaint
#endif
}

//-------------------------------------------------------------
// TestGLCanvas::animate() rotate cube
//-------------------------------------------------------------
void TestGLCanvas::time_tick()
{
	double dt=(double)timer_interval/1000.0;
#ifdef __WXMSW__
	dt*=20;
#endif
	elaped_time=stopwatch.Time()*1e-3;
	fcnt++;
	frame_rate=dcnt/elaped_time;
	angle += 180.0*dt;
	cout << "frame rate:"<< frame_rate<< " count:"<< fcnt << endl;
}

void TestGLCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
     wxPaintDC dc(this);
#ifdef DEBUG_EVENTS
    cout << "ShaderTest::paintGL()" <<endl;
#endif
#ifndef __WXMOTIF__
    if (!GetContext()) return;
#endif

    SetCurrent();

    if(!dcnt)
        InitGL();

    render();
    glFlush();

    // Swap
    SwapBuffers();
    if(running)
    	animate();
    dcnt++;
}

void TestGLCanvas::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);

    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    int w, h;
    GetClientSize(&w, &h);
    if ( GetContext() ){
        SetCurrent();
        reshape(w, h);
    }
#ifdef DEBUG_EVENTS
    cout << "TestGLCanvas::resizeGL("<<w<<","<<h<<")" <<endl;
#endif
}

void TestGLCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
    /* Do nothing, to avoid flashing on MSW */

}

void TestGLCanvas::OnMouseDown( wxMouseEvent& event )
{
	prevMouseX = event.GetX();
	prevMouseY = event.GetY();
}

void TestGLCanvas::OnMouseMove( wxMouseEvent& event )
{
	int x = event.GetX();
	int y = event.GetY();
	if (event.Dragging()) {
		double dx = x - prevMouseX;
		double dy = y - prevMouseY;

		double thetaY = 360.0*dx/(double)width;
		double thetaX = -360.0*dy/(double)height;
		view_rotx += thetaX;
		view_roty += thetaY;
		prevMouseX = x;
		prevMouseY = y;
		Refresh();
	}
}

void TestGLCanvas::init_gears()
{
  static GLfloat pos[4] = {5.0, 5.0, 10.0, 0.0};
  static GLfloat red[4] = {0.8, 0.1, 0.0, 1.0};
  static GLfloat green[4] = {0.0, 0.8, 0.2, 1.0};
  static GLfloat blue[4] = {0.2, 0.2, 1.0, 1.0};
  GLint i;

   view_rotx = 20.0;
   view_roty = 30.0;
   view_rotz = 0.0;
   angle = 0.0;

  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);

  /* make the gears */
  gear1 = glGenLists(1);
  glNewList(gear1, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  gear(1.0, 4.0, 1.0, 20, 0.7);
  glEndList();

  gear2 = glGenLists(1);
  glNewList(gear2, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
  gear(0.5, 2.0, 2.0, 10, 0.7);
  glEndList();

  gear3 = glGenLists(1);
  glNewList(gear3, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
  gear(1.3, 2.0, 0.5, 10, 0.7);
  glEndList();

  glEnable(GL_NORMALIZE);
}

/**

  Draw a gear wheel.  You'll probably want to call this function when
  building a display list since we do a lot of trig here.

  Input:  inner_radius - radius of hole at center
          outer_radius - radius at center of teeth
          width - width of gear
          teeth - number of teeth
          tooth_depth - depth of tooth

 **/

void TestGLCanvas::gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
		GLint teeth, GLfloat tooth_depth) {
	GLint i;
	GLfloat r0, r1, r2;
	GLfloat pitch, da;
	GLfloat u, v, len;

	r0 = inner_radius;
	r1 = outer_radius - tooth_depth / 2.0;
	r2 = outer_radius + tooth_depth / 2.0;

	da = 2.0 * M_PI / teeth / 4.0;

	glShadeModel(GL_FLAT);

	glNormal3f(0.0, 0.0, 1.0);

	/* draw front face */
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++) {
		pitch = i * 2.0 * M_PI / teeth;
		glVertex3f(r0 * cos(pitch), r0 * sin(pitch), width * 0.5);
		glVertex3f(r1 * cos(pitch), r1 * sin(pitch), width * 0.5);
		if (i < teeth) {
			glVertex3f(r0 * cos(pitch), r0 * sin(pitch), width * 0.5);
			glVertex3f(r1 * cos(pitch + 3 * da), r1 * sin(pitch + 3 * da),
					width * 0.5);
		}
	}
	glEnd();

	/* draw front sides of teeth */
	glBegin(GL_QUADS);
	da = 2.0 * M_PI / teeth / 4.0;
	for (i = 0; i < teeth; i++) {
		pitch = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(pitch), r1 * sin(pitch), width * 0.5);
		glVertex3f(r2 * cos(pitch + da), r2 * sin(pitch + da), width * 0.5);
		glVertex3f(r2 * cos(pitch + 2 * da), r2 * sin(pitch + 2 * da), width
				* 0.5);
		glVertex3f(r1 * cos(pitch + 3 * da), r1 * sin(pitch + 3 * da), width
				* 0.5);
	}
	glEnd();

	glNormal3f(0.0, 0.0, -1.0);

	/* draw back face */
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++) {
		pitch = i * 2.0 * M_PI / teeth;
		glVertex3f(r1 * cos(pitch), r1 * sin(pitch), -width * 0.5);
		glVertex3f(r0 * cos(pitch), r0 * sin(pitch), -width * 0.5);
		if (i < teeth) {
			glVertex3f(r1 * cos(pitch + 3 * da), r1 * sin(pitch + 3 * da),
					-width * 0.5);
			glVertex3f(r0 * cos(pitch), r0 * sin(pitch), -width * 0.5);
		}
	}
	glEnd();

	/* draw back sides of teeth */
	glBegin(GL_QUADS);
	da = 2.0 * M_PI / teeth / 4.0;
	for (i = 0; i < teeth; i++) {
		pitch = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(pitch + 3 * da), r1 * sin(pitch + 3 * da), -width
				* 0.5);
		glVertex3f(r2 * cos(pitch + 2 * da), r2 * sin(pitch + 2 * da), -width
				* 0.5);
		glVertex3f(r2 * cos(pitch + da), r2 * sin(pitch + da), -width * 0.5);
		glVertex3f(r1 * cos(pitch), r1 * sin(pitch), -width * 0.5);
	}
	glEnd();

	/* draw outward faces of teeth */
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < teeth; i++) {
		pitch = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(pitch), r1 * sin(pitch), width * 0.5);
		glVertex3f(r1 * cos(pitch), r1 * sin(pitch), -width * 0.5);
		u = r2 * cos(pitch + da) - r1 * cos(pitch);
		v = r2 * sin(pitch + da) - r1 * sin(pitch);
		len = sqrt(u * u + v * v);
		u /= len;
		v /= len;
		glNormal3f(v, -u, 0.0);
		glVertex3f(r2 * cos(pitch + da), r2 * sin(pitch + da), width * 0.5);
		glVertex3f(r2 * cos(pitch + da), r2 * sin(pitch + da), -width * 0.5);
		glNormal3f(cos(pitch), sin(pitch), 0.0);
		glVertex3f(r2 * cos(pitch + 2 * da), r2 * sin(pitch + 2 * da), width
				* 0.5);
		glVertex3f(r2 * cos(pitch + 2 * da), r2 * sin(pitch + 2 * da), -width
				* 0.5);
		u = r1 * cos(pitch + 3 * da) - r2 * cos(pitch + 2 * da);
		v = r1 * sin(pitch + 3 * da) - r2 * sin(pitch + 2 * da);
		glNormal3f(v, -u, 0.0);
		glVertex3f(r1 * cos(pitch + 3 * da), r1 * sin(pitch + 3 * da), width
				* 0.5);
		glVertex3f(r1 * cos(pitch + 3 * da), r1 * sin(pitch + 3 * da), -width
				* 0.5);
		glNormal3f(cos(pitch), sin(pitch), 0.0);
	}

	glVertex3f(r1 * cos(0), r1 * sin(0), width * 0.5);
	glVertex3f(r1 * cos(0), r1 * sin(0), -width * 0.5);

	glEnd();

	glShadeModel(GL_SMOOTH);

	/* draw inside radius cylinder */
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++) {
		pitch = i * 2.0 * M_PI / teeth;
		glNormal3f(-cos(pitch), -sin(pitch), 0.0);
		glVertex3f(r0 * cos(pitch), r0 * sin(pitch), -width * 0.5);
		glVertex3f(r0 * cos(pitch), r0 * sin(pitch), width * 0.5);
	}
	glEnd();

}

void TestGLCanvas::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_gradient();
	draw_gears();
}

void TestGLCanvas::make_gradient(){
	if(pixels)
		::free(pixels);
	int vport[4];
	glGetIntegerv(GL_VIEWPORT,vport);
	int w=vport[2];
	int h=vport[3];
	int size=w*h;
	pixels=(GLubyte*)malloc(size*3);
	double dstart=0.1;
	double dend=1.0-dstart;
	GLubyte *ptr=pixels;

	double a=(dend-dstart)/size;
	double df=dstart;
	for(int i=0;i<h;i++){	// rows (y)
		for(int j=0;j<w;j++){	// cols (x)
			ptr[0]=ptr[1]=ptr[2]=(GLubyte)(df*255);
			df+=a;
			ptr+=3;
		}
	}
}
void TestGLCanvas::draw_gradient()
{
	if(show_gradient && pixels){
		int vport[4];
		glGetIntegerv(GL_VIEWPORT,vport);

		glDepthMask(GL_FALSE);
		glDrawPixels(vport[2]-1,vport[3]-1,GL_RGB, GL_UNSIGNED_BYTE,pixels);
		glDepthMask(GL_TRUE);
	}
}

void TestGLCanvas::draw_gears()
{

  glPushMatrix();
    glRotated(view_rotx, 1.0, 0.0, 0.0);
    glRotated(view_roty, 0.0, 1.0, 0.0);
    glRotated(view_rotz, 0.0, 0.0, 1.0);

    glPushMatrix();
      glTranslated(-3.0, -2.0, 0.0);
      glRotated(angle, 0.0, 0.0, 1.0);
      glCallList(gear1);
    glPopMatrix();

    glPushMatrix();
      glTranslated(3.1, -2.0, 0.0);
      glRotated(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
      glCallList(gear2);
    glPopMatrix();

    glPushMatrix();
      glTranslated(-3.1, 4.2, 0.0);
      glRotated(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
      glCallList(gear3);
    glPopMatrix();

  glPopMatrix();
}

