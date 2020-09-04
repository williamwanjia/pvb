/*=========================================================================

  Copyright 2004 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

=========================================================================*/

/*========================================================================
 For general information about using VTK and Qt, see:
 http://www.trolltech.com/products/3rdparty/vtksupport.html
=========================================================================*/

/*========================================================================
 !!! WARNING for those who want to contribute code to this file.
 !!! If you use a commercial edition of Qt, you can modify this code.
 !!! If you use an open source version of Qt, you are free to modify
 !!! and use this code within the guidelines of the GPL license.
 !!! Unfortunately, you cannot contribute the changes back into this
 !!! file.  Doing so creates a conflict between the GPL and BSD-like VTK
 !!! license.
=========================================================================*/

// .NAME QVTKWidget - Display a vtkRenderWindow in a Qt's QWidget.
// .SECTION Description
// QVTKWidget provides a way to display VTK data in a Qt widget.

#ifndef Q_VTK_WIDGET_H
#define Q_VTK_WIDGET_H

#include <QOpenGLWidget>

#include "vtkNew.h"                // needed for vtkNew
#include "vtkSmartPointer.h"       // needed for vtkSmartPointer

#include <QtCore/QObject>

#include "vtkRenderWindowInteractor.h"
#include "vtkCommand.h"

class QOpenGLDebugLogger;
class QOpenGLFramebufferObject;
class QVTKInteractor;
class QVTKInteractorInternal;
class QVTKInteractorAdapter;
class QVTKWidgetObserver;
class vtkGenericOpenGLRenderWindow;
class vtkRenderWindowInteractor;
class QEvent;
class QSignalMapper;
class QTimer;

class QVTKWidget : public QOpenGLWidget
{
  Q_OBJECT
  typedef QOpenGLWidget Superclass;
public:
  QVTKWidget(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
  ~QVTKWidget() override;

  //@{
  /**
   * Get/Set the currently used vtkGenericOpenGLRenderWindow.
   */
  void SetRenderWindow(vtkGenericOpenGLRenderWindow* win);
  void SetRenderWindow(vtkRenderWindow* win);
  virtual vtkRenderWindow* GetRenderWindow();
  //@}

  /**
   * Get the QVTKInteractor that was either created by default or set by the user.
   */
  virtual QVTKInteractor* GetInteractor();

  /**
   * Sets up vtkRenderWindow ivars using QSurfaceFormat.
   */
  static void copyFromFormat(const QSurfaceFormat& format, vtkRenderWindow* win);

  /**
   * Using the vtkRenderWindow, setup QSurfaceFormat.
   */
  static void copyToFormat(vtkRenderWindow* win, QSurfaceFormat& format);

  /**
   * Returns a typical QSurfaceFormat suitable for most applications using
   * QVTKWidget. Note that this is not the QSurfaceFormat that gets used
   * if none is specified. That is set using `QSurfaceFormat::setDefaultFormat`.
   */
  static QSurfaceFormat defaultFormat();

  /**
   * Enable or disable support for HiDPI displays.
   */
  virtual void setEnableHiDPI(bool enable);

signals:
  /**
   * This signal will be emitted whenever a mouse event occurs within the QVTK window.
   */
  void mouseEvent(QMouseEvent* event);

protected slots:
  /**
   * Called as a response to `QOpenGLContext::aboutToBeDestroyed`. This may be
   * called anytime during the widget lifecycle. We need to release any OpenGL
   * resources allocated in VTK work in this method.
   */
  virtual void cleanupContext();

private slots:
  /**
   * recreates the FBO used for VTK rendering.
   */
  void recreateFBO();

  /**
   * called before the render window starts to render. We ensure that this->FBO
   * is bound and ready to use.
   */
  void startEventCallback();

protected:
  bool event(QEvent* evt) Q_DECL_OVERRIDE;
  void initializeGL() Q_DECL_OVERRIDE;
  void resizeGL(int w, int h) Q_DECL_OVERRIDE;
  void paintGL() Q_DECL_OVERRIDE;

  void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseDoubleClickEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

  /**
   * This method is called to indicate that vtkRenderWindow needs to reinitialize
   * itself before the next render (done in QVTKWidget::paintGL).
   * This is needed when the context gets recreated
   * or the default FrameBufferObject gets recreated, for example.
   */
  void requireRenderWindowInitialization();

  /**
   * This method may be called in `paintGL` to request VTK to do a render i.e.
   * trigger render on the render window via its interactor.
   *
   * It will return true if render (or an equivalent action) was performed to
   * update the frame buffer made available to VTK for rendering with latest
   * rendering.
   *
   * Default implementation never returns false. However, subclasses can return
   * false to indicate to QVTKWidget that it cannot generate a reasonable
   * image to be displayed in QVTKWidget. In which case, the `paintGL`
   * call will return leaving the `defaultFramebufferObject` untouched.
   *
   * Since by default `QOpenGLWidget::UpdateBehavior` is set to
   * QOpenGLWidget::PartialUpdate, this means whatever was rendered in the frame
   * buffer in most recent successful call will be preserved, unless the widget
   * was forced to recreate the FBO as a result of resize or screen change.
   *
   * @sa Section @ref RenderAndPaint.
   */
  virtual bool renderVTK();

protected:
  vtkSmartPointer<vtkGenericOpenGLRenderWindow> RenderWindow;
  QVTKInteractorAdapter* InteractorAdaptor;

  bool EnableHiDPI;
  int OriginalDPI;

private:
  Q_DISABLE_COPY(QVTKWidget);

  /**
   * Called when vtkCommand::WindowFrameEvent is fired by the
   * vtkGenericOpenGLRenderWindow.
   */
  void windowFrameEventCallback();

  QOpenGLFramebufferObject* FBO;
  bool InPaintGL;
  bool DoVTKRenderInPaintGL;
  vtkNew<QVTKWidgetObserver> Observer;
  friend class QVTKWidgetObserver;
  QOpenGLDebugLogger* Logger;
};

// internal class, do not use
class QVTKInteractorInternal : public QObject
{
  Q_OBJECT
public:
  QVTKInteractorInternal(QVTKInteractor* p);
  ~QVTKInteractorInternal() override;
public Q_SLOTS:
  void TimerEvent(int id);
public:
  QSignalMapper* SignalMapper;
  typedef std::map<int, QTimer*> TimerMap;
  TimerMap Timers;
  QVTKInteractor* Parent;
};

/**
 * @class QVTKInteractor
 * @brief - an interactor for QVTKWidget (and QVTKWiget).
 *
 * QVTKInteractor handles relaying Qt events to VTK.
 * @sa QVTKWidget
 */

class QVTKInteractor : public vtkRenderWindowInteractor
{
public:
  static QVTKInteractor* New();
  vtkTypeMacro(QVTKInteractor,vtkRenderWindowInteractor);

  /**
   * Enum for additional event types supported.
   * These events can be picked up by command observers on the interactor.
   */
  enum vtkCustomEvents
  {
    ContextMenuEvent = vtkCommand::UserEvent + 100,
    DragEnterEvent,
    DragMoveEvent,
    DragLeaveEvent,
    DropEvent
  };

  /**
   * Overloaded terminate app, which does nothing in Qt.
   * Use qApp->exit() instead.
   */
  void TerminateApp() override;

  /**
   * Overloaded start method does nothing.
   * Use qApp->exec() instead.
   */
  void Start() override;
  void Initialize() override;

  /**
   * Start listening events on 3DConnexion device.
   */
  virtual void StartListening();

  /**
   * Stop listening events on 3DConnexion device.
   */
  virtual void StopListening();

  /**
   * timer event slot
   */
  virtual void TimerEvent(int timerId);

protected:
  // constructor
  QVTKInteractor();
  // destructor
  ~QVTKInteractor() override;

  // create a Qt Timer
  int InternalCreateTimer(int timerId, int timerType, unsigned long duration) override;
  // destroy a Qt Timer
  int InternalDestroyTimer(int platformTimerId) override;

private:

  QVTKInteractorInternal* Internal;

  // unimplemented copy
  QVTKInteractor(const QVTKInteractor&);
  // unimplemented operator=
  void operator=(const QVTKInteractor&);

};

// .NAME QVTKInteractorAdapter - A QEvent translator.
// .SECTION Description
// QVTKInteractorAdapter translates QEvents and send them to a
// vtkRenderWindowInteractor.
class QVTKInteractorAdapter : public QObject
{
  Q_OBJECT
public:
  // Description:
  // Constructor: takes QObject parent
  QVTKInteractorAdapter(QObject* parent);

  // Description:
  // Destructor
  ~QVTKInteractorAdapter() override;

  // Description:
  // Set the device pixel ration, this defaults to 1, but in Qt 5 can be 2.
  void SetDevicePixelRatio(int ratio, vtkRenderWindowInteractor* iren = nullptr);
  int GetDevicePixelRatio() { return this->DevicePixelRatio; }

  // Description:
  // Process a QEvent and send it to the interactor
  // returns whether the event was recognized and processed
  bool ProcessEvent(QEvent* e, vtkRenderWindowInteractor* iren);

protected:
  int AccumulatedDelta;
  int DevicePixelRatio;
};

#endif


