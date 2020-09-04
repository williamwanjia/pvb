/* 
 * tkAppInit.c --
 *
 *      Provides a default version of the Tcl_AppInit procedure for
 *      use in wish and similar Tk-based applications.
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "pvdefine.h"
#include "tk.h"

#include "vtkConfigure.h"
#include "vtkSystemIncludes.h"
#include "vtkToolkits.h"

#ifndef vtkTkAppInitConfigure_h
#define vtkTkAppInitConfigure_h

/* Where the VTK Tcl packages can be found */
#define VTK_INSTALL_TCL_DIR "@VTK_INSTALL_TCL_DIR@"

#endif


#ifdef VTK_TCL_TK_COPY_SUPPORT_LIBRARY
#include <sys/stat.h>
#endif

#ifdef VTK_USE_TK
# include "vtkTk.h"
#else
# include "vtkTcl.h"
#endif

#include "vtkTclUtil.h"


/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *      This is the main program for the application.
 *
 * Results:
 *      None: Tk_Main never returns here, so this procedure never
 *      returns either.
 *
 * Side effects:
 *      Whatever the application does.
 *
 *----------------------------------------------------------------------
 */
int
xmain(int argc, char **argv)
{
  ios::sync_with_stdio();
  vtkTclApplicationInitExecutable(argc, argv);
#ifdef xxxVTK_USE_RENDERING
  Tk_Main(argc, argv, Tcl_AppInit);
#else
  Tcl_Main(argc, argv, Tcl_AppInit);
#endif
  return 0;                  /* Needed only to prevent compiler warning. */
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *      This procedure performs application-specific initialization.
 *      Most applications, especially those that incorporate additional
 *      packages, will have their own version of this procedure.
 *
 * Results:
 *      Returns a standard Tcl completion code, and leaves an error
 *      message in interp->result if an error occurs.
 *
 * Side effects:
 *      Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */
void help()
{
}

int Tcl_AppInit(Tcl_Interp *interp)
{
#ifdef __CYGWIN__
  Tcl_SetVar(interp, "tclDefaultLibrary", "/usr/share/tcl" TCL_VERSION, TCL_GLOBAL_ONLY);
#endif

  // Help Tcl find the Tcl/Tk helper files.
  const char* relative_dirs[] =
    {
      "TclTk/lib",
      ".." VTK_INSTALL_TCL_DIR,
      0
    };
  vtkTclApplicationInitTclTk(interp, relative_dirs);

  if (Tcl_Init(interp) == TCL_ERROR)
  {
    return TCL_ERROR;
  }

#ifdef VTK_USE_TK
  if (Tk_Init(interp) == TCL_ERROR)
  {
    return TCL_ERROR;
  }
#endif

#ifndef VTK_BUILD_SHARED_LIBS
# include "vtktcl_static_packages.h"
#endif

#ifdef VTK_EXTRA_TCL_INIT
  VTK_EXTRA_TCL_INIT;
#endif

  /*
   * Append path to VTK packages to auto_path
   */
  static char script[] =
    "foreach dir [list "
    " [file dirname [file dirname [info nameofexecutable]]]"
#if defined(CMAKE_INTDIR)
    " [file join [file dirname [file dirname [file dirname [info nameofexecutable]]]] Wrapping Tcl " CMAKE_INTDIR "]"
#else
    " [file join [file dirname [file dirname [info nameofexecutable]]] Wrapping Tcl]"
#endif
    " ] {\n"
    "  if {[file isdirectory \"$dir\"]} {\n"
    "    lappend auto_path $dir\n"
    "  }\n"
    "}\n"
    "rename package package.orig\n"
    "proc package {args} {\n"
    "  if {[catch {set package_res [eval package.orig $args]} catch_res]} {\n"
    "    global errorInfo env\n"
    "    if {[lindex $args 0] == \"require\"} {\n"
    "      set expecting {can\'t find package vtk}\n"
    "      if {![string compare -length [string length $expecting] $catch_res $expecting]} {\n"
    "        set msg {The Tcl interpreter was probably not able to find the"
    " VTK packages.  Please check that your TCLLIBPATH environment variable"
    " includes the path to your VTK Tcl directory.  You might find it under"
    " your VTK binary directory in Wrapping/Tcl, or under your"
    " site-specific installation directory.}\n"
    "        if {[info exists env(TCLLIBPATH)]} {\n"
    "          append msg \"  The TCLLIBPATH current value is: $env(TCLLIBPATH).\"\n"
    "        }\n"
    "        set errorInfo \"$msg  The TCLLIBPATH variable is a set of"
    " space separated paths (hence, path containing spaces should be"
    " surrounded by quotes first). Windows users should use forward"
    " (Unix-style) slashes \'/\' instead of the usual backward slashes. "
    " More information can be found in the Wrapping/Tcl/README source"
    " file (also available online at"
    " http://www.vtk.org/cgi-bin/cvsweb.cgi/~checkout~/VTK/Wrapping/Tcl/README).\n"
    "$errorInfo\"\n"
    "      }\n"
    "    }\n"
    "  error $catch_res $errorInfo\n"
    "  }\n"
    "  return $package_res\n"
    "}\n"
    "if $tcl_interactive {\n"
    "puts {Enter: \"package require vtk\" to load VTK commands}\n"
    "}\n";
  Tcl_Eval(interp, script);

  /*
   * Specify a user-specific startup file to invoke if the application
   * is run interactively.  Typically the startup file is "~/.apprc"
   * where "app" is the name of the application.  If this line is deleted
   * then no user-specific startup file will be run under any conditions.
   */

  Tcl_SetVar(interp,
             (char *) "tcl_rcFileName",
             (char *) "~/.vtkrc",
             TCL_GLOBAL_ONLY);
  return TCL_OK;
}





