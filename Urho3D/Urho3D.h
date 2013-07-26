//
// Copyright (c) 2008-2013 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "Application.h"

using namespace Urho3D;

namespace Urho3D
{
    class ScriptFile;
}

/// Urho3D script shell application, which runs a script specified on the command line.
class Urho : public Application
{
    OBJECT(Urho);
    
public:
    /// Construct.
    Urho(Context* context);
    
    /// Setup before engine initialization. Verify that a script file has been specified.
    virtual int Setup();
    /// Startup after engine initialization. Load the script and execute its start function.
    virtual int Start();
    /// Cleanup after the main loop. Run the script's stop function if it exists.
    virtual int Stop();
    
private:
    /// Show last error message and set the exit code for error exit.
    void ErrorExit();
    /// Handle reload start of the script file.
    void HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData);
    /// Handle reload success of the script file.
    void HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData);
    /// Handle reload failure of the script file.
    void HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData);
    
    /// Script file name.
    String scriptFileName_;
    /// Script file.
    SharedPtr<ScriptFile> scriptFile_;
    /// Application exit code.
    int exitCode_;
};