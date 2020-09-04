/*=========================================================================

  Program:   Visualization Toolkit
  Module:    QVTKWidget.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "QVTKWidget.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLTexture>
#include <QPointer>
#include <QScopedValueRollback>
#include <QtDebug>

#include "vtkCommand.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"

#include <QEvent>
#include <QSignalMapper>
#include <QTimer>
#include <QResizeEvent>

#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkRenderWindow.h"

// #define DEBUG_QVTKOPENGL_WIDGET
#ifdef DEBUG_QVTKOPENGL_WIDGET
#define vtkQVTKWidgetDebugMacro(msg)                                                         \
  cout << this << ": " msg << endl;                                                                \
  if (this->Logger)                                                                                \
  {                                                                                                \
    this->Logger->logMessage(                                                                      \
      QOpenGLDebugMessage::createApplicationMessage(QStringLiteral("QVTKWidget::" msg)));    \
  }
#else
#define vtkQVTKWidgetDebugMacro(x)
#endif

// function to get VTK keysyms from ascii characters
static const char* ascii_to_key_sym(int);
// function to get VTK keysyms from Qt keys
static const char* qt_key_to_key_sym(Qt::Key, Qt::KeyboardModifiers modifiers);

class QVTKWidgetObserver : public vtkCommand
{
public:
  static QVTKWidgetObserver* New() { return new QVTKWidgetObserver(); }
  vtkTypeMacro(QVTKWidgetObserver, vtkCommand);

  void SetTarget(QVTKWidget* target) { this->Target = target; }

  void Execute(vtkObject*, unsigned long eventId, void* callData) override
  {
    if (this->Target)
    {
      switch (eventId)
      {
        case vtkCommand::WindowMakeCurrentEvent:
          {
            // We do not call QOpenGLWidget::makeCurrent() as that also makes the
            // frame buffer object used by QOpenGLWidget active. This can have
            // unintended side effects when MakeCurrent gets called in a
            // render-pass, for example. We should only be making the context
            // active. To do that, we use this trick. We rely on the
            // QOpenGLContext have been called makeCurrent() previously so we
            // can get to the surface that was used to do that. We simply
            // reactivate on that surface.
            QOpenGLContext* ctxt = this->Target->context();
            QSurface* surface = ctxt? ctxt->surface() : nullptr;
            if (surface)
            {
              ctxt->makeCurrent(surface);
            }
            Q_ASSERT(ctxt == nullptr || surface != nullptr);
          }
          break;

        case vtkCommand::WindowIsCurrentEvent:
        {
          bool& cstatus = *reinterpret_cast<bool*>(callData);
          cstatus = (QOpenGLContext::currentContext() == this->Target->context());
        }
        break;

        case vtkCommand::WindowFrameEvent:
          this->Target->windowFrameEventCallback();
          break;

        case vtkCommand::StartEvent:
          VTK_FALLTHROUGH;
        case vtkCommand::StartPickEvent:
          this->Target->startEventCallback();
          break;
      }
    }
  }

protected:
  QVTKWidgetObserver() {}
  ~QVTKWidgetObserver() override {}
  QPointer<QVTKWidget> Target;
};

//-----------------------------------------------------------------------------
QVTKWidget::QVTKWidget(QWidget* parentWdg, Qt::WindowFlags f)
  : Superclass(parentWdg, f)
  , InteractorAdaptor(nullptr)
  , EnableHiDPI(false)
  , OriginalDPI(0)
  , FBO(nullptr)
  , InPaintGL(false)
  , DoVTKRenderInPaintGL(false)
  , Logger(nullptr)
{
  this->Observer->SetTarget(this);

  // default to strong focus
  this->setFocusPolicy(Qt::StrongFocus);

  this->setUpdateBehavior(QOpenGLWidget::PartialUpdate);

  this->InteractorAdaptor = new QVTKInteractorAdapter(this);
  this->InteractorAdaptor->SetDevicePixelRatio(this->devicePixelRatio());

  this->setMouseTracking(true);

  // QOpenGLWidget::resized() is triggered when the default FBO in QOpenGLWidget is recreated.
  // We use the same signal to recreate our FBO.
  this->connect(this, SIGNAL(resized()), SLOT(recreateFBO()));
}

//-----------------------------------------------------------------------------
QVTKWidget::~QVTKWidget()
{
  vtkQVTKWidgetDebugMacro("~QVTKWidget");
  // essential to cleanup context so that the render window finalizes and
  // releases any graphics resources it may have allocated.
  this->cleanupContext();
  this->SetRenderWindow(static_cast<vtkGenericOpenGLRenderWindow*>(nullptr));
  this->Observer->SetTarget(nullptr);
  delete this->InteractorAdaptor;
  delete this->Logger;
}

//-----------------------------------------------------------------------------
void QVTKWidget::SetRenderWindow(vtkRenderWindow* win)
{
  vtkGenericOpenGLRenderWindow* gwin = vtkGenericOpenGLRenderWindow::SafeDownCast(win);
  this->SetRenderWindow(gwin);
  if (gwin == nullptr && win != nullptr)
  {
    qDebug() << "QVTKWidget requires a `vtkGenericOpenGLRenderWindow`. `"
             << win->GetClassName() << "` is not supported.";
  }
}

//-----------------------------------------------------------------------------
void QVTKWidget::SetRenderWindow(vtkGenericOpenGLRenderWindow* win)
{
  if (this->RenderWindow == win)
  {
    return;
  }

  if (this->RenderWindow)
  {
    this->RenderWindow->RemoveObserver(this->Observer);
    this->RenderWindow->SetReadyForRendering(false);
  }
  this->RenderWindow = win;
  this->requireRenderWindowInitialization();
  if (this->RenderWindow)
  {
    // set this to 0 to reinitialize it before setting the RenderWindow DPI
    this->OriginalDPI = 0;

    // if an interactor wasn't provided, we'll make one by default
    if (!this->RenderWindow->GetInteractor())
    {
      // create a default interactor
      vtkNew<QVTKInteractor> iren;
      this->RenderWindow->SetInteractor(iren);
      iren->Initialize();

      // now set the default style
      vtkNew<vtkInteractorStyleTrackballCamera> style;
      iren->SetInteractorStyle(style);
    }

    this->RenderWindow->AddObserver(vtkCommand::WindowMakeCurrentEvent, this->Observer);
    this->RenderWindow->AddObserver(vtkCommand::WindowIsCurrentEvent, this->Observer);
    this->RenderWindow->AddObserver(vtkCommand::WindowFrameEvent, this->Observer);
    this->RenderWindow->AddObserver(vtkCommand::StartEvent, this->Observer);
    this->RenderWindow->AddObserver(vtkCommand::StartPickEvent, this->Observer);

    if (this->FBO)
    {
      this->makeCurrent();
      this->recreateFBO();
    }
  }
}

//-----------------------------------------------------------------------------
void QVTKWidget::startEventCallback()
{
  vtkQVTKWidgetDebugMacro("startEventCallback");
  this->makeCurrent();
  if (this->FBO)
  {
    // ensure that before vtkRenderWindow starts to render, we activate the FBO
    // to render into. VTK code can be a bit lax with it. This just ensures that
    // we have the FBO activated.
    this->FBO->bind();
  }
}

//-----------------------------------------------------------------------------
vtkRenderWindow* QVTKWidget::GetRenderWindow()
{
  return this->RenderWindow;
}

//-----------------------------------------------------------------------------
QVTKInteractor* QVTKWidget::GetInteractor()
{
  return QVTKInteractor::SafeDownCast(this->GetRenderWindow()->GetInteractor());
}

//-----------------------------------------------------------------------------
void QVTKWidget::copyFromFormat(const QSurfaceFormat& format, vtkRenderWindow* win)
{
  if (vtkOpenGLRenderWindow* oglWin = vtkOpenGLRenderWindow::SafeDownCast(win))
  {
    oglWin->SetStereoCapableWindow(format.stereo() ? 1 : 0);
    // samples may not be correct if format is obtained from
    // QOpenGLWidget::format() after the context is created. That's because
    // QOpenGLWidget always created context to samples=0.
    oglWin->SetMultiSamples(format.samples());
    oglWin->SetStencilCapable(format.stencilBufferSize() > 0);
  }
}

//-----------------------------------------------------------------------------
void QVTKWidget::copyToFormat(vtkRenderWindow* win, QSurfaceFormat& format)
{
  if (vtkOpenGLRenderWindow* oglWin = vtkOpenGLRenderWindow::SafeDownCast(win))
  {
    format.setStereo(oglWin->GetStereoCapableWindow());
    format.setSamples(oglWin->GetMultiSamples());
    format.setStencilBufferSize(oglWin->GetStencilCapable() ? 8 : 0);
  }
}

//-----------------------------------------------------------------------------
QSurfaceFormat QVTKWidget::defaultFormat()
{
  QSurfaceFormat fmt;
  fmt.setRenderableType(QSurfaceFormat::OpenGL);
  fmt.setVersion(3, 2);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  fmt.setRedBufferSize(1);
  fmt.setGreenBufferSize(1);
  fmt.setBlueBufferSize(1);
  fmt.setDepthBufferSize(1);
  fmt.setStencilBufferSize(0);
  fmt.setAlphaBufferSize(1);
  fmt.setStereo(false);
  fmt.setSamples(vtkOpenGLRenderWindow::GetGlobalMaximumNumberOfMultiSamples());
#ifdef DEBUG_QVTKOPENGL_WIDGET
  fmt.setOption(QSurfaceFormat::DebugContext);
#endif
  return fmt;
}

//-----------------------------------------------------------------------------
void QVTKWidget::setEnableHiDPI(bool enable)
{
  this->EnableHiDPI = enable;

  if (this->RenderWindow)
  {
    if (this->OriginalDPI == 0)
    {
      this->OriginalDPI = this->RenderWindow->GetDPI();
    }
    if (this->EnableHiDPI)
    {
      this->RenderWindow->SetDPI(this->OriginalDPI * this->devicePixelRatio());
    }
    else
    {
      this->RenderWindow->SetDPI(this->OriginalDPI);
    }
  }
}

//-----------------------------------------------------------------------------
void QVTKWidget::recreateFBO()
{
  vtkQVTKWidgetDebugMacro("recreateFBO");
  delete this->FBO;
  this->FBO = nullptr;
  if (!this->RenderWindow)
  {
    return;
  }

  // Since QVTKWidget::initializeGL() cannot set multi-samples
  // state on the RenderWindow correctly, we do it here.
  QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
  GLint samples;
  f->glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);

  // Some graphics drivers report the number of samples as 1 when
  // multisampling is off. Set the number of samples to 0 in this case.
  samples = samples > 1 ? samples : 0;
  this->RenderWindow->SetMultiSamples(static_cast<int>(samples));

  QOpenGLFramebufferObjectFormat format;
  format.setAttachment(QOpenGLFramebufferObject::Depth);
  format.setSamples(samples);

  const int devicePixelRatio_ = this->devicePixelRatio();
  const QSize widgetSize = this->size();
  const QSize deviceSize = widgetSize * devicePixelRatio_;

  // This is as good an opportunity as any to communicate size to the render
  // window.
  this->InteractorAdaptor->SetDevicePixelRatio(devicePixelRatio_);
  if (vtkRenderWindowInteractor* iren = this->RenderWindow->GetInteractor())
  {
    iren->SetSize(deviceSize.width(), deviceSize.height());
  }
  this->RenderWindow->SetSize(deviceSize.width(), deviceSize.height());
  this->RenderWindow->SetPosition(this->x() * devicePixelRatio_, this->y() * devicePixelRatio_);

  // Set screen size on render window.
  const QRect screenGeometry = QApplication::desktop()->screenGeometry(this);
  this->RenderWindow->SetScreenSize(screenGeometry.width(), screenGeometry.height());

  this->FBO = new QOpenGLFramebufferObject(deviceSize, format);
  this->FBO->bind();
  this->RenderWindow->SetForceMaximumHardwareLineWidth(1);
  this->RenderWindow->SetReadyForRendering(true);
  this->RenderWindow->InitializeFromCurrentContext();

  this->setEnableHiDPI(this->EnableHiDPI);

  // Since the context or frame buffer was recreated, if a paintGL call ensues,
  // we need to ensure we're requesting VTK to render.
  this->DoVTKRenderInPaintGL = true;

  // Clear to ensure that an uninitialized framebuffer is never displayed.
  f->glDisable(GL_SCISSOR_TEST);
  f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  f->glClear(GL_COLOR_BUFFER_BIT);
}

//-----------------------------------------------------------------------------
void QVTKWidget::initializeGL()
{
  this->Superclass::initializeGL();

#ifdef DEBUG_QVTKOPENGL_WIDGET
  delete this->Logger;
  this->Logger = new QOpenGLDebugLogger(this);
  this->Logger->initialize(); // initializes in the current context.
#endif

  vtkQVTKWidgetDebugMacro("initializeGL");
  if (this->RenderWindow)
  {
    // use QSurfaceFormat for the widget, update ivars on the vtkRenderWindow.
    QVTKWidget::copyFromFormat(this->format(), this->RenderWindow);
    // When a QOpenGLWidget is told to use a QSurfaceFormat with samples > 0,
    // QOpenGLWidget doesn't actually create a context with multi-samples and
    // internally changes the QSurfaceFormat to be samples=0. Thus, we can't
    // rely on the QSurfaceFormat to indicate to us if multisampling is being
    // used. We should use glGetRenderbufferParameteriv(..) to get
    // GL_RENDERBUFFER_SAMPLES to determine the samples used. This is done by
    // in recreateFBO().
  }
  this->connect(
    this->context(), SIGNAL(aboutToBeDestroyed()), SLOT(cleanupContext()), Qt::UniqueConnection);
  this->requireRenderWindowInitialization();
}

//-----------------------------------------------------------------------------
void QVTKWidget::requireRenderWindowInitialization()
{
  if (this->RenderWindow)
  {
    this->RenderWindow->SetReadyForRendering(false);
  }
}

//-----------------------------------------------------------------------------
void QVTKWidget::resizeGL(int w, int h)
{
  vtkQVTKWidgetDebugMacro("resizeGL");
  this->Superclass::resizeGL(w, h);
}

//-----------------------------------------------------------------------------
void QVTKWidget::paintGL()
{
  if (this->InPaintGL)
  {
    return;
  }

  if (!this->RenderWindow)
  {
    return;
  }

  if (!this->FBO
    || this->FBO->handle() != this->RenderWindow->GetDefaultFrameBufferId())
  {
    this->recreateFBO();
  }

  QScopedValueRollback<bool> var(this->InPaintGL, true);
  this->Superclass::paintGL();

  if (this->DoVTKRenderInPaintGL && !this->renderVTK())
  {
    vtkQVTKWidgetDebugMacro("paintGL:skipped-renderVTK");
    // This should be very rare, but it's conceivable that subclasses of
    // QVTKWidget are simply not ready to do a
    // render on VTK render window when widget is being painted.
    // Leave the buffer unchanged.
    return;
  }

  // We just did a render, if we needed it. Turn the flag off.
  this->DoVTKRenderInPaintGL = false;

  // If render was triggered by above calls, that may change the current context
  // due to things like progress events triggering updates on other widgets
  // (e.g. progress bar). Hence we need to make sure to call makeCurrent()
  // before proceeding with blit-ing.
  this->makeCurrent();

  // blit from this->FBO to QOpenGLWidget's FBO.
  vtkQVTKWidgetDebugMacro("paintGL::blit-to-defaultFBO");
  QOpenGLFunctions_3_2_Core* f =
    QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_2_Core>();
  if (f)
  {
    f->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->defaultFramebufferObject());
    f->glDrawBuffer(GL_COLOR_ATTACHMENT0);

    f->glBindFramebuffer(GL_READ_FRAMEBUFFER, this->FBO->handle());
    f->glReadBuffer(GL_COLOR_ATTACHMENT0);
    f->glDisable(GL_SCISSOR_TEST); // Scissor affects glBindFramebuffer.
    f->glBlitFramebuffer(0, 0, this->RenderWindow->GetSize()[0], this->RenderWindow->GetSize()[1],
      0, 0, this->RenderWindow->GetSize()[0], this->RenderWindow->GetSize()[1], GL_COLOR_BUFFER_BIT,
      GL_NEAREST);

    // now clear alpha otherwise we end up blending the rendering with
    // background windows in certain cases. It happens on OsX
    // (if QSurfaceFormat::alphaBufferSize() > 0) or when using Mesa on Linux
    // (see paraview/paraview#17159).
    GLboolean colorMask[4];
    f->glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
    f->glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);

    GLfloat clearColor[4];
    f->glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
    f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);

    f->glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
    f->glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
  }
}

//-----------------------------------------------------------------------------
void QVTKWidget::cleanupContext()
{
  // logger gets uninitialized when this gets called. We get errors from
  // QOpenGLDebugLogger if logMessage is called here. So we just destroy the
  // logger.
  delete this->Logger;
  this->Logger = nullptr;

  vtkQVTKWidgetDebugMacro("cleanupContext");

  // QOpenGLWidget says when this slot is called, the context may not be current
  // and hence is a good practice to make it so.
  this->makeCurrent();
  if (this->RenderWindow)
  {
    if (this->FBO)
    {
      this->FBO->bind();
    }
    this->RenderWindow->Finalize();
    this->RenderWindow->SetReadyForRendering(false);
  }
  delete this->FBO;
  this->FBO = nullptr;
  this->requireRenderWindowInitialization();
}

//-----------------------------------------------------------------------------
void QVTKWidget::windowFrameEventCallback()
{
  Q_ASSERT(this->RenderWindow);
  vtkQVTKWidgetDebugMacro("frame");

  if (!this->InPaintGL)
  {
    // Handing vtkOpenGLRenderWindow::Frame is tricky. VTK code traditionally
    // calls `Frame` to indicate that VTK is done rendering 1 frame. Now, when
    // that happens, should we tell Qt to update the widget -- that's the
    // question? In general, yes, but sometimes VTK does things in the
    // background i.e. back buffer without wanting to update the front buffer
    // e.g. when making selections. In that case, we don't want to update Qt
    // widget either, since whatever it did was not meant to be visible.
    // To handle that, we check if vtkOpenGLRenderWindow::SwapBuffers is true,
    // and request an update only when it is.
    if (this->RenderWindow->GetSwapBuffers() || this->RenderWindow->GetDoubleBuffer() == 0)
    {
      // Means that the vtkRenderWindow rendered outside a paintGL call. That can
      // happen when application code call vtkRenderWindow::Render() directly,
      // instead of calling QVTKWidget::update() or letting Qt update the
      // widget. In that case, since QOpenGLWidget rendering into an offscreen
      // FBO, the result still needs to be composed by Qt widget stack. We request
      // that using `update()`.
      vtkQVTKWidgetDebugMacro("update");
      this->update();

      this->DoVTKRenderInPaintGL = false;
    }
    else
    {
      vtkQVTKWidgetDebugMacro("buffer bad -- do not show");

      // Since this->FBO right now is garbage, if paint event is received before
      // a Render request is made on the render window, we will have to Render
      // explicitly.
      this->DoVTKRenderInPaintGL = true;
    }
  }
}

//-----------------------------------------------------------------------------
bool QVTKWidget::renderVTK()
{
  vtkQVTKWidgetDebugMacro("renderVTK");
  Q_ASSERT(this->FBO);
  Q_ASSERT(this->FBO->handle() == this->RenderWindow->GetDefaultFrameBufferId());

  // Bind the FBO we'll be rendering into. This may not be needed, since VTK will
  // bind it anyways, but we'll be extra cautious.
  this->FBO->bind();

  vtkRenderWindowInteractor* iren = this->RenderWindow ? this->RenderWindow->GetInteractor() : nullptr;
  if (iren)
  {
    iren->Render();
  }
  else if (this->RenderWindow)
  {
    this->RenderWindow->Render();
  }
  else
  {
    // no render window set, just fill with white.
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);
  }
  return true;
}

//-----------------------------------------------------------------------------
bool QVTKWidget::event(QEvent* evt)
{
  switch (evt->type())
  {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
      // skip events that are explicitly handled by overrides to avoid duplicate
      // calls to InteractorAdaptor->ProcessEvent().
      break;

    case QEvent::Resize:
      // we don't let QVTKInteractorAdapter process resize since we handle it
      // in this->recreateFBO().
      break;

    default:
      if (this->RenderWindow && this->RenderWindow->GetInteractor())
      {
        this->InteractorAdaptor->ProcessEvent(evt, this->RenderWindow->GetInteractor());
      }
  }
  return this->Superclass::event(evt);
}

//-----------------------------------------------------------------------------
void QVTKWidget::mousePressEvent(QMouseEvent* event)
{
  emit mouseEvent(event);

  if (this->RenderWindow && this->RenderWindow->GetInteractor())
  {
    this->InteractorAdaptor->ProcessEvent(event,
                                          this->RenderWindow->GetInteractor());
  }
}

//-----------------------------------------------------------------------------
void QVTKWidget::mouseMoveEvent(QMouseEvent* event)
{
  emit mouseEvent(event);

  if (this->RenderWindow && this->RenderWindow->GetInteractor())
  {
    this->InteractorAdaptor->ProcessEvent(event,
                                          this->RenderWindow->GetInteractor());
  }
}

//-----------------------------------------------------------------------------
void QVTKWidget::mouseReleaseEvent(QMouseEvent* event)
{
  emit mouseEvent(event);

  if (this->RenderWindow && this->RenderWindow->GetInteractor())
  {
    this->InteractorAdaptor->ProcessEvent(event,
                                          this->RenderWindow->GetInteractor());
  }
}

//-----------------------------------------------------------------------------
void QVTKWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  emit mouseEvent(event);

  if (this->RenderWindow && this->RenderWindow->GetInteractor())
  {
    this->InteractorAdaptor->ProcessEvent(event,
                                          this->RenderWindow->GetInteractor());
  }
}

/*! allocation method for Qt/VTK interactor
 */
vtkStandardNewMacro(QVTKInteractor);

/*! constructor for Qt/VTK interactor
 */
QVTKInteractor::QVTKInteractor()
{
  this->Internal = new QVTKInteractorInternal(this);
}

void QVTKInteractor::Initialize()
{
  this->Initialized = 1;
  this->Enable();
}


/*! start method for interactor
 */
void QVTKInteractor::Start()
{
  vtkErrorMacro(<<"QVTKInteractor cannot control the event loop.");
}

/*! terminate the application
 */
void QVTKInteractor::TerminateApp()
{
  // we are in a GUI so let's terminate the GUI the normal way
  //qApp->exit();
}

// ----------------------------------------------------------------------------
void QVTKInteractor::StartListening()
{
}

// ----------------------------------------------------------------------------
void QVTKInteractor::StopListening()
{
}


/*! handle timer event
 */
void QVTKInteractor::TimerEvent(int timerId)
{
  if ( !this->GetEnabled() )
  {
    return;
  }
  this->InvokeEvent(vtkCommand::TimerEvent, (void*)&timerId);

  if(this->IsOneShotTimer(timerId))
  {
    this->DestroyTimer(timerId);  // 'cause our Qt timers are always repeating
  }
}

/*! constructor
 */
QVTKInteractor::~QVTKInteractor()
{
  delete this->Internal;
}

/*! create Qt timer with an interval of 10 msec.
 */
int QVTKInteractor::InternalCreateTimer(int timerId, int vtkNotUsed(timerType), unsigned long duration)
{
  QTimer* timer = new QTimer(this->Internal);
  timer->start(duration);
  this->Internal->SignalMapper->setMapping(timer, timerId);
  QObject::connect(timer, SIGNAL(timeout()), this->Internal->SignalMapper, SLOT(map()));
  int platformTimerId = timer->timerId();
  this->Internal->Timers.insert(QVTKInteractorInternal::TimerMap::value_type(platformTimerId, timer));
  return platformTimerId;
}

/*! destroy timer
 */
int QVTKInteractor::InternalDestroyTimer(int platformTimerId)
{
  QVTKInteractorInternal::TimerMap::iterator iter = this->Internal->Timers.find(platformTimerId);
  if(iter != this->Internal->Timers.end())
  {
    iter->second->stop();
    iter->second->deleteLater();
    this->Internal->Timers.erase(iter);
    return 1;
  }
  return 0;
}

QVTKInteractorInternal::QVTKInteractorInternal(QVTKInteractor* p)
  : Parent(p)
{
  this->SignalMapper = new QSignalMapper(this);
  QObject::connect(this->SignalMapper, SIGNAL(mapped(int)), this, SLOT(TimerEvent(int)) );
}

QVTKInteractorInternal::~QVTKInteractorInternal()
{
}

void QVTKInteractorInternal::TimerEvent(int id)
{
  Parent->TimerEvent(id);
}

QVTKInteractorAdapter::QVTKInteractorAdapter(QObject* parentObject)
  : QObject(parentObject), AccumulatedDelta(0), DevicePixelRatio(1)
{
}

QVTKInteractorAdapter::~QVTKInteractorAdapter()
{
}

void QVTKInteractorAdapter::SetDevicePixelRatio(int ratio, vtkRenderWindowInteractor* iren)
{
  if (ratio != DevicePixelRatio)
  {
    if (iren)
    {
      int tmp[2];
      iren->GetSize(tmp);
      if (ratio == 1)
      {
        iren->SetSize(tmp[0] / 2, tmp[1] / 2);
      }
      else if (ratio == 2)
      {
        iren->SetSize(tmp[0] * 2, tmp[1] * 2);
      }
    }
    this->DevicePixelRatio = ratio;
  }
}

bool QVTKInteractorAdapter::ProcessEvent(QEvent* e, vtkRenderWindowInteractor* iren)
{
  if(iren == nullptr || e == nullptr)
    return false;

  const QEvent::Type t = e->type();

  if(t == QEvent::Resize)
  {
    QResizeEvent* e2 = static_cast<QResizeEvent*>(e);
    QSize size = e2->size();
    iren->SetSize(size.width() * this->DevicePixelRatio,
                  size.height() * this->DevicePixelRatio);
    iren->InvokeEvent(vtkCommand::ConfigureEvent, e2);
    return true;
  }

  if(t == QEvent::FocusIn)
  {
    // For 3DConnexion devices:
    QVTKInteractor* qiren = QVTKInteractor::SafeDownCast(iren);
    if(qiren)
    {
      qiren->StartListening();
    }
    return true;
  }

  if(t == QEvent::FocusOut)
  {
    // For 3DConnexion devices:
    QVTKInteractor* qiren = QVTKInteractor::SafeDownCast(iren);
    if(qiren)
    {
      qiren->StopListening();
    }
    return true;
  }

  // the following events only happen if the interactor is enabled
  if(!iren->GetEnabled())
    return false;

  if(t == QEvent::MouseButtonPress ||
     t == QEvent::MouseButtonRelease ||
     t == QEvent::MouseButtonDblClick ||
     t == QEvent::MouseMove)
  {
    QMouseEvent* e2 = static_cast<QMouseEvent*>(e);

    // give interactor the event information
    iren->SetEventInformationFlipY(e2->x() * this->DevicePixelRatio,
                                   e2->y() * this->DevicePixelRatio,
                                (e2->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                (e2->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0,
                                0,
                                e2->type() == QEvent::MouseButtonDblClick ? 1 : 0);
    iren->SetAltKey((e2->modifiers() & Qt::AltModifier) > 0 ? 1 : 0);

    if(t == QEvent::MouseMove)
    {
      iren->InvokeEvent(vtkCommand::MouseMoveEvent, e2);
    }
    else if(t == QEvent::MouseButtonPress || t == QEvent::MouseButtonDblClick)
    {
      switch(e2->button())
      {
        case Qt::LeftButton:
          iren->InvokeEvent(vtkCommand::LeftButtonPressEvent, e2);
          break;

        case Qt::MidButton:
          iren->InvokeEvent(vtkCommand::MiddleButtonPressEvent, e2);
          break;

        case Qt::RightButton:
          iren->InvokeEvent(vtkCommand::RightButtonPressEvent, e2);
          break;

        default:
          break;
      }
    }
    else if(t == QEvent::MouseButtonRelease)
    {
      switch(e2->button())
      {
        case Qt::LeftButton:
          iren->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, e2);
          break;

        case Qt::MidButton:
          iren->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, e2);
          break;

        case Qt::RightButton:
          iren->InvokeEvent(vtkCommand::RightButtonReleaseEvent, e2);
          break;

        default:
          break;
      }
    }
    return true;
  }
  if (t == QEvent::TouchBegin ||
      t == QEvent::TouchUpdate ||
      t == QEvent::TouchEnd)
  {
    QTouchEvent* e2 = dynamic_cast<QTouchEvent*>(e);
    foreach (const QTouchEvent::TouchPoint& point, e2->touchPoints())
    {
      if (point.id() >= VTKI_MAX_POINTERS)
      {
        break;
      }
      // give interactor the event information
      iren->SetEventInformationFlipY(point.pos().x() * this->DevicePixelRatio,
                                     point.pos().y() * this->DevicePixelRatio,
                                      (e2->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                      (e2->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0,
                                      0,0,0, point.id());
    }
    foreach (const QTouchEvent::TouchPoint& point, e2->touchPoints())
    {
      if (point.id() >= VTKI_MAX_POINTERS)
      {
        break;
      }
      iren->SetPointerIndex(point.id());
      if (point.state() & Qt::TouchPointReleased)
      {
        iren->InvokeEvent(vtkCommand::LeftButtonReleaseEvent,nullptr);
      }
      if (point.state() & Qt::TouchPointPressed)
      {
        iren->InvokeEvent(vtkCommand::LeftButtonPressEvent,nullptr);
      }
      if (point.state() & Qt::TouchPointMoved)
      {
        iren->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);
      }
    }
    e2->accept();
    return true;
  }

  if(t == QEvent::Enter)
  {
    iren->InvokeEvent(vtkCommand::EnterEvent, e);
    return true;
  }

  if(t == QEvent::Leave)
  {
    iren->InvokeEvent(vtkCommand::LeaveEvent, e);
    return true;
  }

  if(t == QEvent::KeyPress || t == QEvent::KeyRelease)
  {
    QKeyEvent* e2 = static_cast<QKeyEvent*>(e);

    // get key and keysym information
    int ascii_key = e2->text().length() ? e2->text().unicode()->toLatin1() : 0;
    const char* keysym = ascii_to_key_sym(ascii_key);
    if(!keysym ||
       e2->modifiers() == Qt::KeypadModifier)
    {
      // get virtual keys
      keysym = qt_key_to_key_sym(static_cast<Qt::Key>(e2->key()),
                                 e2->modifiers());
    }

    if(!keysym)
    {
      keysym = "None";
    }

    // give interactor event information
    iren->SetKeyEventInformation(
      (e2->modifiers() & Qt::ControlModifier),
      (e2->modifiers() & Qt::ShiftModifier),
      ascii_key, e2->count(), keysym);
    iren->SetAltKey((e2->modifiers() & Qt::AltModifier) > 0 ? 1 : 0);

    if(t == QEvent::KeyPress)
    {
      // invoke vtk event
      iren->InvokeEvent(vtkCommand::KeyPressEvent, e2);

      // invoke char event only for ascii characters
      if(ascii_key)
      {
        iren->InvokeEvent(vtkCommand::CharEvent, e2);
      }
    }
    else
    {
      iren->InvokeEvent(vtkCommand::KeyReleaseEvent, e2);
    }
    return true;
  }

  if(t == QEvent::Wheel)
  {
    QWheelEvent* e2 = static_cast<QWheelEvent*>(e);

    iren->SetEventInformationFlipY(e2->x() * this->DevicePixelRatio,
                                   e2->y() * this->DevicePixelRatio,
                               (e2->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                               (e2->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0);
    iren->SetAltKey((e2->modifiers() & Qt::AltModifier) > 0 ? 1 : 0);

#if QT_VERSION >= 0x050000
    this->AccumulatedDelta += e2->angleDelta().y();
#else
    this->AccumulatedDelta += e2->delta();
#endif
    const int threshold = 120;

    // invoke vtk event when accumulated delta passes the threshold
    if(this->AccumulatedDelta >= threshold)
    {
      iren->InvokeEvent(vtkCommand::MouseWheelForwardEvent, e2);
      this->AccumulatedDelta = 0;
    }
    else if(this->AccumulatedDelta <= -threshold)
    {
      iren->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, e2);
      this->AccumulatedDelta = 0;
    }
    return true;
  }

  if(t == QEvent::ContextMenu)
  {
    QContextMenuEvent* e2 = static_cast<QContextMenuEvent*>(e);

    // give interactor the event information
    iren->SetEventInformationFlipY(e2->x() * this->DevicePixelRatio,
                                   e2->y() * this->DevicePixelRatio,
                               (e2->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                               (e2->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0);
    iren->SetAltKey((e2->modifiers() & Qt::AltModifier) > 0 ? 1 : 0);

    // invoke event and pass qt event for additional data as well
    iren->InvokeEvent(QVTKInteractor::ContextMenuEvent, e2);

    return true;
  }

  if(t == QEvent::DragEnter)
  {
    QDragEnterEvent* e2 = static_cast<QDragEnterEvent*>(e);

    // invoke event and pass qt event for additional data as well
    iren->InvokeEvent(QVTKInteractor::DragEnterEvent, e2);

    return true;
  }

  if(t == QEvent::DragLeave)
  {
    QDragLeaveEvent* e2 = static_cast<QDragLeaveEvent*>(e);

    // invoke event and pass qt event for additional data as well
    iren->InvokeEvent(QVTKInteractor::DragLeaveEvent, e2);

    return true;
  }

  if(t == QEvent::DragMove)
  {
    QDragMoveEvent* e2 = static_cast<QDragMoveEvent*>(e);

    // give interactor the event information
    iren->SetEventInformationFlipY(e2->pos().x() * this->DevicePixelRatio,
                                   e2->pos().y() * this->DevicePixelRatio);

    // invoke event and pass qt event for additional data as well
    iren->InvokeEvent(QVTKInteractor::DragMoveEvent, e2);
    return true;
  }

  if(t == QEvent::Drop)
  {
    QDropEvent* e2 = static_cast<QDropEvent*>(e);

    // give interactor the event information
    iren->SetEventInformationFlipY(e2->pos().x() * this->DevicePixelRatio,
                                   e2->pos().y() * this->DevicePixelRatio);

    // invoke event and pass qt event for additional data as well
    iren->InvokeEvent(QVTKInteractor::DropEvent, e2);
    return true;
  }

  return false;
}

// ***** keysym stuff below  *****

static const char *AsciiToKeySymTable[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, "Tab", 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "space", "exclam", "quotedbl", "numbersign",
  "dollar", "percent", "ampersand", "quoteright",
  "parenleft", "parenright", "asterisk", "plus",
  "comma", "minus", "period", "slash",
  "0", "1", "2", "3", "4", "5", "6", "7",
  "8", "9", "colon", "semicolon", "less", "equal", "greater", "question",
  "at", "A", "B", "C", "D", "E", "F", "G",
  "H", "I", "J", "K", "L", "M", "N", "O",
  "P", "Q", "R", "S", "T", "U", "V", "W",
  "X", "Y", "Z", "bracketleft",
  "backslash", "bracketright", "asciicircum", "underscore",
  "quoteleft", "a", "b", "c", "d", "e", "f", "g",
  "h", "i", "j", "k", "l", "m", "n", "o",
  "p", "q", "r", "s", "t", "u", "v", "w",
  "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "Delete",
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const char* ascii_to_key_sym(int i)
{
  if(i >= 0)
  {
    return AsciiToKeySymTable[i];
  }
  return 0;
}

#define QVTK_HANDLE(x,y) \
  case x : \
    ret = y; \
    break;

#define QVTK_HANDLE_KEYPAD(x, y, z) \
  case x : \
    ret = (modifiers & Qt::KeypadModifier) ? (y) : (z); \
    break;

const char* qt_key_to_key_sym(Qt::Key i, Qt::KeyboardModifiers modifiers)
{
  const char* ret = 0;
  switch(i)
  {
    // Cancel
    QVTK_HANDLE(Qt::Key_Backspace, "BackSpace")
      QVTK_HANDLE(Qt::Key_Tab, "Tab")
      QVTK_HANDLE(Qt::Key_Backtab, "Tab")
      QVTK_HANDLE(Qt::Key_Clear, "Clear")
      QVTK_HANDLE(Qt::Key_Return, "Return")
      QVTK_HANDLE(Qt::Key_Enter, "Return")
      QVTK_HANDLE(Qt::Key_Shift, "Shift_L")
      QVTK_HANDLE(Qt::Key_Control, "Control_L")
      QVTK_HANDLE(Qt::Key_Alt, "Alt_L")
      QVTK_HANDLE(Qt::Key_Pause, "Pause")
      QVTK_HANDLE(Qt::Key_CapsLock, "Caps_Lock")
      QVTK_HANDLE(Qt::Key_Escape, "Escape")
      QVTK_HANDLE(Qt::Key_Space, "space")
      QVTK_HANDLE(Qt::Key_PageUp, "Prior")
      QVTK_HANDLE(Qt::Key_PageDown, "Next")
      QVTK_HANDLE(Qt::Key_End, "End")
      QVTK_HANDLE(Qt::Key_Home, "Home")
      QVTK_HANDLE(Qt::Key_Left, "Left")
      QVTK_HANDLE(Qt::Key_Up, "Up")
      QVTK_HANDLE(Qt::Key_Right, "Right")
      QVTK_HANDLE(Qt::Key_Down, "Down")
      QVTK_HANDLE(Qt::Key_Select, "Select")
      QVTK_HANDLE(Qt::Key_Execute, "Execute")
      QVTK_HANDLE(Qt::Key_SysReq, "Snapshot")
      QVTK_HANDLE(Qt::Key_Insert, "Insert")
      QVTK_HANDLE(Qt::Key_Delete, "Delete")
      QVTK_HANDLE(Qt::Key_Help, "Help")
      QVTK_HANDLE_KEYPAD(Qt::Key_0, "KP_0", "0")
      QVTK_HANDLE_KEYPAD(Qt::Key_1, "KP_1", "1")
      QVTK_HANDLE_KEYPAD(Qt::Key_2, "KP_2", "2")
      QVTK_HANDLE_KEYPAD(Qt::Key_3, "KP_3", "3")
      QVTK_HANDLE_KEYPAD(Qt::Key_4, "KP_4", "4")
      QVTK_HANDLE_KEYPAD(Qt::Key_5, "KP_5", "5")
      QVTK_HANDLE_KEYPAD(Qt::Key_6, "KP_6", "6")
      QVTK_HANDLE_KEYPAD(Qt::Key_7, "KP_7", "7")
      QVTK_HANDLE_KEYPAD(Qt::Key_8, "KP_8", "8")
      QVTK_HANDLE_KEYPAD(Qt::Key_9, "KP_9", "9")
      QVTK_HANDLE(Qt::Key_A, "a")
      QVTK_HANDLE(Qt::Key_B, "b")
      QVTK_HANDLE(Qt::Key_C, "c")
      QVTK_HANDLE(Qt::Key_D, "d")
      QVTK_HANDLE(Qt::Key_E, "e")
      QVTK_HANDLE(Qt::Key_F, "f")
      QVTK_HANDLE(Qt::Key_G, "g")
      QVTK_HANDLE(Qt::Key_H, "h")
      QVTK_HANDLE(Qt::Key_I, "i")
      QVTK_HANDLE(Qt::Key_J, "h")
      QVTK_HANDLE(Qt::Key_K, "k")
      QVTK_HANDLE(Qt::Key_L, "l")
      QVTK_HANDLE(Qt::Key_M, "m")
      QVTK_HANDLE(Qt::Key_N, "n")
      QVTK_HANDLE(Qt::Key_O, "o")
      QVTK_HANDLE(Qt::Key_P, "p")
      QVTK_HANDLE(Qt::Key_Q, "q")
      QVTK_HANDLE(Qt::Key_R, "r")
      QVTK_HANDLE(Qt::Key_S, "s")
      QVTK_HANDLE(Qt::Key_T, "t")
      QVTK_HANDLE(Qt::Key_U, "u")
      QVTK_HANDLE(Qt::Key_V, "v")
      QVTK_HANDLE(Qt::Key_W, "w")
      QVTK_HANDLE(Qt::Key_X, "x")
      QVTK_HANDLE(Qt::Key_Y, "y")
      QVTK_HANDLE(Qt::Key_Z, "z")
      QVTK_HANDLE(Qt::Key_Asterisk, "asterisk")
      QVTK_HANDLE(Qt::Key_Plus, "plus")
      QVTK_HANDLE(Qt::Key_Bar, "bar")
      QVTK_HANDLE(Qt::Key_Minus, "minus")
      QVTK_HANDLE(Qt::Key_Period, "period")
      QVTK_HANDLE(Qt::Key_Slash, "slash")
      QVTK_HANDLE(Qt::Key_F1, "F1")
      QVTK_HANDLE(Qt::Key_F2, "F2")
      QVTK_HANDLE(Qt::Key_F3, "F3")
      QVTK_HANDLE(Qt::Key_F4, "F4")
      QVTK_HANDLE(Qt::Key_F5, "F5")
      QVTK_HANDLE(Qt::Key_F6, "F6")
      QVTK_HANDLE(Qt::Key_F7, "F7")
      QVTK_HANDLE(Qt::Key_F8, "F8")
      QVTK_HANDLE(Qt::Key_F9, "F9")
      QVTK_HANDLE(Qt::Key_F10, "F10")
      QVTK_HANDLE(Qt::Key_F11, "F11")
      QVTK_HANDLE(Qt::Key_F12, "F12")
      QVTK_HANDLE(Qt::Key_F13, "F13")
      QVTK_HANDLE(Qt::Key_F14, "F14")
      QVTK_HANDLE(Qt::Key_F15, "F15")
      QVTK_HANDLE(Qt::Key_F16, "F16")
      QVTK_HANDLE(Qt::Key_F17, "F17")
      QVTK_HANDLE(Qt::Key_F18, "F18")
      QVTK_HANDLE(Qt::Key_F19, "F19")
      QVTK_HANDLE(Qt::Key_F20, "F20")
      QVTK_HANDLE(Qt::Key_F21, "F21")
      QVTK_HANDLE(Qt::Key_F22, "F22")
      QVTK_HANDLE(Qt::Key_F23, "F23")
      QVTK_HANDLE(Qt::Key_F24, "F24")
      QVTK_HANDLE(Qt::Key_NumLock, "Num_Lock")
      QVTK_HANDLE(Qt::Key_ScrollLock, "Scroll_Lock")

      default:
    break;
  }
  return ret;
}
