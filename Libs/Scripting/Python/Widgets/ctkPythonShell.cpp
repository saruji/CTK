/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.commontk.org/LICENSE

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/
/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

//#include <vtkPython.h> // python first

// Qt includes
#include <QCoreApplication>
#include <QResizeEvent>
#include <QScrollBar>
#include <QStringListModel>
#include <QTextCharFormat>
#include <QVBoxLayout>

// PythonQt includes
#include <PythonQt.h>
#include <PythonQtObjectPtr.h>

// CTK includes
#include <ctkConsoleWidget.h>
#include <ctkAbstractPythonManager.h>
#include "ctkPythonShell.h"

#ifdef __GNUC__
// Disable warnings related to Python macros and functions
// See http://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
// Note: Ideally the incriminated functions and macros should be fixed upstream ...
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

//----------------------------------------------------------------------------
class ctkPythonShellCompleter : public ctkConsoleWidgetCompleter
{
public:
  ctkPythonShellCompleter(ctkPythonShell& p) : Parent(p)
    {
    this->setParent(&p);
    }

  virtual void updateCompletionModel(const QString& completion)
    {
    // Start by clearing the model
    this->setModel(0);

    // Don't try to complete the empty string
    if (completion.isEmpty())
      {
      return;
      }

    // Search backward through the string for usable characters
    QString textToComplete;
    for (int i = completion.length()-1; i >= 0; --i)
      {
      QChar c = completion.at(i);
      if (c.isLetterOrNumber() || c == '.' || c == '_')
        {
        textToComplete.prepend(c);
        }
      else
        {
        break;
        }
      }

    // Split the string at the last dot, if one exists
    QString lookup;
    QString compareText = textToComplete;
    int dot = compareText.lastIndexOf('.');
    if (dot != -1)
      {
      lookup = compareText.mid(0, dot);
      compareText = compareText.mid(dot+1);
      }

    // Lookup python names
    QStringList attrs;
    if (!lookup.isEmpty() || !compareText.isEmpty())
      {
      attrs = Parent.getPythonAttributes(lookup);
      }

    // Initialize the completion model
    if (!attrs.isEmpty())
      {
      this->setCompletionMode(QCompleter::PopupCompletion);
      this->setModel(new QStringListModel(attrs, this));
      this->setCaseSensitivity(Qt::CaseInsensitive);
      this->setCompletionPrefix(compareText.toLower());
      this->popup()->setCurrentIndex(this->completionModel()->index(0, 0));
      }
    }
  ctkPythonShell& Parent;
};


/////////////////////////////////////////////////////////////////////////
// ctkPythonShell::pqImplementation

struct ctkPythonShell::pqImplementation
{
  pqImplementation(QWidget* _parent, ctkAbstractPythonManager* pythonManager)
    : Console(_parent), PythonManager(pythonManager), MultilineStatement(false),
    InteractiveConsole(0)
  {
  }

//----------------------------------------------------------------------------
//   void initialize(int argc, char* argv[])
//   {
//     // Setup Python's interactive prompts
//     PyObject* ps1 = PySys_GetObject(const_cast<char*>("ps1"));
//     if(!ps1)
//       {
//       PySys_SetObject(const_cast<char*>("ps1"), ps1 = PyString_FromString(">>> "));
//       Py_XDECREF(ps1);
//       }
// 
//     PyObject* ps2 = PySys_GetObject(const_cast<char*>("ps2"));
//     if(!ps2)
//       {
//       PySys_SetObject(const_cast<char*>("ps2"), ps2 = PyString_FromString("... "));
//       Py_XDECREF(ps2);
//       }
//     this->MultilineStatement = false;
//   }

//----------------------------------------------------------------------------
  ~pqImplementation()
  {
//     this->destroyInterpretor();
  }

//----------------------------------------------------------------------------
//   void destroyInterpretor()
//     {
//     if (this->Interpreter)
//       {
//       QTextCharFormat format = this->Console.getFormat();
//       format.setForeground(QColor(255, 0, 0));
//       this->Console.setFormat(format);
//       this->Console.printString("\n... restarting ...\n");
//       format.setForeground(QColor(0, 0, 0));
//       this->Console.setFormat(format);
// 
//       this->Interpreter->MakeCurrent();
// 
//       // Restore Python's original stdout and stderr
//       PySys_SetObject(const_cast<char*>("stdout"), PySys_GetObject(const_cast<char*>("__stdout__")));
//       PySys_SetObject(const_cast<char*>("stderr"), PySys_GetObject(const_cast<char*>("__stderr__")));
//       this->Interpreter->ReleaseControl();
//       this->Interpreter->Delete();
//       }
//     this->Interpreter = 0;
//     }

  //----------------------------------------------------------------------------
  void initializeInteractiveConsole()
  {
    // set up the code.InteractiveConsole instance that we'll use.
    const char* code =
      "import code\n"
      "__ctkConsole=code.InteractiveConsole(locals())\n";
    PyRun_SimpleString(code);

    // Now get the reference to __ctkConsole and save the pointer.
    PyObject* main_module = PyImport_AddModule("__main__");
    PyObject* global_dict = PyModule_GetDict(main_module);
    this->InteractiveConsole = PyDict_GetItemString(
      global_dict, "__ctkConsole");
    if (!this->InteractiveConsole)
      {
      qCritical("Failed to locate the InteractiveConsole object.");
      }
  }


//----------------------------------------------------------------------------
  bool push(const QString& code)
  {
    bool ret_value = false;

    QString buffer = code;
    // The embedded python interpreter cannot handle DOS line-endings, see
    // http://sourceforge.net/tracker/?group_id=5470&atid=105470&func=detail&aid=1167922
    buffer.remove('\r');

    PyObject *res = PyObject_CallMethod(this->InteractiveConsole,
                                        const_cast<char*>("push"),
                                        const_cast<char*>("z"),
                                        buffer.toAscii().data());
    if (res)
      {
      int status = 0;
      if (PyArg_Parse(res, "i", &status))
        {
        ret_value = (status > 0);
        }
      Py_DECREF(res);
      }
    return ret_value;
  }
//----------------------------------------------------------------------------
void resetBuffer()
  {
  if (this->InteractiveConsole)
    {
    //this->MakeCurrent();
    const char* code = "__ctkConsole.resetbuffer()\n";
    PyRun_SimpleString(code);
    //this->ReleaseControl();
    }
  }

//----------------------------------------------------------------------------
  void executeCommand(const QString& command)
  {
    this->MultilineStatement = this->push(command);
//    if (command.length())
//      {
//      Q_ASSERT(this->PythonManager);
//      this->PythonManager->executeString(command);
//      }
  }
  
//----------------------------------------------------------------------------
  void promptForInput(const QString& indent=QString())
  {
    QTextCharFormat format = this->Console.getFormat();
    format.setForeground(QColor(0, 0, 0));
    this->Console.setFormat(format);

//     this->Interpreter->MakeCurrent();
    if(!this->MultilineStatement)
      {
      this->Console.prompt(">>> ");
      //this->Console.prompt(
      //  PyString_AsString(PySys_GetObject(const_cast<char*>("ps1"))));
      }
    else
      {
      this->Console.prompt("... ");
      //this->Console.prompt(
      //  PyString_AsString(PySys_GetObject(const_cast<char*>("ps2"))));
      }
    this->Console.printCommand(indent);
//     this->Interpreter->ReleaseControl();
  }

  /// Provides a console for gathering user input and displaying 
  /// Python output
  ctkConsoleWidget Console;

  ctkAbstractPythonManager* PythonManager;

  /// Indicates if the last statement processes was incomplete.
  bool MultilineStatement;

  PyObject* InteractiveConsole;
};

/////////////////////////////////////////////////////////////////////////
// ctkPythonShell

//----------------------------------------------------------------------------
ctkPythonShell::ctkPythonShell(ctkAbstractPythonManager* pythonManager, QWidget* _parent):
  Superclass(_parent),
  Implementation(new pqImplementation(this, pythonManager))
{
  // Layout UI
  QVBoxLayout* const boxLayout = new QVBoxLayout(this);
  boxLayout->setMargin(0);
  boxLayout->addWidget(&this->Implementation->Console);

  this->setObjectName("pythonShell");

  this->setFocusProxy(&this->Implementation->Console);

  ctkPythonShellCompleter* completer = new ctkPythonShellCompleter(*this);
  this->Implementation->Console.setCompleter(completer);
  
  QObject::connect(
    &this->Implementation->Console, SIGNAL(executeCommand(const QString&)), 
    this, SLOT(onExecuteCommand(const QString&)));

  // The call to mainContext() ensures that python has been initialized.
  Q_ASSERT(this->Implementation->PythonManager);
  this->Implementation->PythonManager->mainContext();
  this->Implementation->initializeInteractiveConsole();

  QTextCharFormat format = this->Implementation->Console.getFormat();
  format.setForeground(QColor(0, 0, 255));
  this->Implementation->Console.setFormat(format);
  this->Implementation->Console.printString(
    QString("Python %1 on %2\n").arg(Py_GetVersion()).arg(Py_GetPlatform()));
  this->promptForInput();

  Q_ASSERT(PythonQt::self());

  this->connect(PythonQt::self(), SIGNAL(pythonStdOut(const QString&)),
                SLOT(printStdout(const QString&)));
  this->connect(PythonQt::self(), SIGNAL(pythonStdErr(const QString&)),
                SLOT(printStderr(const QString&)));
}

//----------------------------------------------------------------------------
ctkPythonShell::~ctkPythonShell()
{
  delete this->Implementation;
}

//----------------------------------------------------------------------------
void ctkPythonShell::clear()
{
  this->Implementation->Console.clear();
  this->Implementation->promptForInput();
}

//----------------------------------------------------------------------------
void ctkPythonShell::executeScript(const QString& script)
{
  Q_UNUSED(script);
  
  this->printStdout("\n");
  emit this->executing(true);
//   this->Implementation->Interpreter->RunSimpleString(
//     script.toAscii().data());
  emit this->executing(false);
  this->Implementation->promptForInput();
}

//----------------------------------------------------------------------------
QStringList ctkPythonShell::getPythonAttributes(const QString& pythonVariableName)
{
//   this->makeCurrent();

  Q_ASSERT(PyThreadState_GET()->interp);
  PyObject* dict = PyImport_GetModuleDict();
  PyObject* object = PyDict_GetItemString(dict, "__main__");
  Py_INCREF(object);


  if (!pythonVariableName.isEmpty())
    {
    QStringList tmpNames = pythonVariableName.split('.');
    for (int i = 0; i < tmpNames.size() && object; ++i)
      {
      QByteArray tmpName = tmpNames.at(i).toLatin1();
      PyObject* prevObj = object;
      if (PyDict_Check(object))
        {
        object = PyDict_GetItemString(object, tmpName.data());
        Py_XINCREF(object);
        }
      else
        {
        object = PyObject_GetAttrString(object, tmpName.data());
        }
      Py_DECREF(prevObj);
      }
    PyErr_Clear();
    }

  QStringList results;
  if (object)
    {
    PyObject* keys = PyObject_Dir(object);
    if (keys)
      {
      PyObject* key;
      PyObject* value;
      QString keystr;
      int nKeys = PyList_Size(keys);
      for (int i = 0; i < nKeys; ++i)
        {
        key = PyList_GetItem(keys, i);
        value = PyObject_GetAttr(object, key);
        if (!value)
          {
          continue;
          }

        results << PyString_AsString(key);
        Py_DECREF(value);
        }
      Py_DECREF(keys);
      }
    Py_DECREF(object);
    }

//   this->releaseControl();
  return results;
}

//----------------------------------------------------------------------------
void ctkPythonShell::printStdout(const QString& text)
{
  QTextCharFormat format = this->Implementation->Console.getFormat();
  format.setForeground(QColor(0, 150, 0));
  this->Implementation->Console.setFormat(format);
  
  this->Implementation->Console.printString(text);
  
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

//----------------------------------------------------------------------------
void ctkPythonShell::printMessage(const QString& text)
{
  QTextCharFormat format = this->Implementation->Console.getFormat();
  format.setForeground(QColor(0, 0, 150));
  this->Implementation->Console.setFormat(format);
  
  this->Implementation->Console.printString(text);
}

//----------------------------------------------------------------------------
void ctkPythonShell::printStderr(const QString& text)
{
  QTextCharFormat format = this->Implementation->Console.getFormat();
  format.setForeground(QColor(255, 0, 0));
  this->Implementation->Console.setFormat(format);
  
  this->Implementation->Console.printString(text);
  
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

//----------------------------------------------------------------------------
void ctkPythonShell::onExecuteCommand(const QString& Command)
{
  QString command = Command;
  command.replace(QRegExp("\\s*$"), "");
  this->internalExecuteCommand(command);

  // Find the indent for the command.
  QRegExp regExp("^(\\s+)");
  QString indent;
  if (regExp.indexIn(command) != -1)
    {
    indent = regExp.cap(1);
    }
  this->Implementation->promptForInput(indent);

  emit this->executedCommand(Command);
}

//----------------------------------------------------------------------------
void ctkPythonShell::promptForInput()
{
  this->Implementation->promptForInput();
}

//----------------------------------------------------------------------------
void ctkPythonShell::internalExecuteCommand(const QString& command)
{
  emit this->executing(true);  
  this->Implementation->executeCommand(command);
  emit this->executing(false);
}
