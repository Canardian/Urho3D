//
// Copyright (c) 2008-2014 the Urho3D project.
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

#include "Precompiled.h"
#include "Context.h"
#include "File.h"
#include "IOEvents.h"
#include "Log.h"
#include "Mutex.h"
#include "ProcessUtils.h"
#include "Timer.h"

#include <cstdio>

#ifdef ANDROID
#include <android/log.h>
#endif
#ifdef IOS
extern "C" void SDL_IOS_LogMessage(const char* message);
#endif

#include "DebugNew.h"

namespace Urho3D
{

const char* logLevelPrefixes[] =
{
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    0
};

static Log* logInstance = 0;

Log::Log(Context* context) :
    Object(context),
#ifdef _DEBUG
    level_(LOG_DEBUG),
#else
    level_(LOG_INFO),
#endif
    timeStamp_(true),
    inWrite_(false),
    quiet_(false)
{
    logInstance = this;
}

Log::~Log()
{
    logInstance = 0;
}

void Log::Open(const String& fileName)
{
    #if !defined(ANDROID) && !defined(IOS)
    if (fileName.Empty())
        return;
    if (logFile_ && logFile_->IsOpen())
    {
        if (logFile_->GetName() == fileName)
            return;
        else
            Close();
    }

    logFile_ = new File(context_);
    if (logFile_->Open(fileName, FILE_WRITE))
        Write(LOG_INFO, "Opened log file " + fileName);
    else
    {
        logFile_.Reset();
        Write(LOG_ERROR, "Failed to create log file " + fileName);
    }
    #endif
}

void Log::Close()
{
    #if !defined(ANDROID) && !defined(IOS)
    if (logFile_ && logFile_->IsOpen())
    {
        logFile_->Close();
        logFile_.Reset();
    }
    #endif
}

void Log::SetLevel(int level)
{
    assert(level >= LOG_DEBUG && level < LOG_NONE);

    level_ = level;
}

void Log::SetTimeStamp(bool enable)
{
    timeStamp_ = enable;
}

void Log::SetQuiet(bool quiet)
{
    quiet_ = quiet;
}

void Log::Write(int level, const String& message)
{
    assert(level >= LOG_DEBUG && level < LOG_NONE);

    // Do not log if message level excluded or if currently sending a log event
    if (!logInstance || logInstance->level_ > level || logInstance->inWrite_)
        return;

    String formattedMessage = logLevelPrefixes[level];
    formattedMessage += ": " + message;
    logInstance->lastMessage_ = message;

    if (logInstance->timeStamp_)
        formattedMessage = "[" + Time::GetTimeStamp() + "] " + formattedMessage;

    #if defined(ANDROID)
    int androidLevel = ANDROID_LOG_DEBUG + level;
    __android_log_print(androidLevel, "Urho3D", "%s", message.CString());
    #elif defined(IOS)
    SDL_IOS_LogMessage(message.CString());
    #else
    if (logInstance->quiet_)
    {
        // If in quiet mode, still print the error message to the standard error stream
        if (level == LOG_ERROR)
            PrintUnicodeLine(formattedMessage, true);
    }
    else
        PrintUnicodeLine(formattedMessage, level == LOG_ERROR);
    #endif

    if (logInstance->logFile_)
    {
        logInstance->logFile_->WriteLine(formattedMessage);
        logInstance->logFile_->Flush();
    }

    logInstance->inWrite_ = true;

    using namespace LogMessage;

    VariantMap& eventData = logInstance->GetEventDataMap();
    eventData[P_MESSAGE] = formattedMessage;
    eventData[P_LEVEL] = level;
    logInstance->SendEvent(E_LOGMESSAGE, eventData);

    logInstance->inWrite_ = false;
}

void Log::WriteRaw(const String& message, bool error)
{
    // Prevent recursion during log event
    if (!logInstance || logInstance->inWrite_)
        return;

    logInstance->lastMessage_ = message;

    #if defined(ANDROID)
    if (logInstance->quiet_)
    {
        if (error)
            __android_log_print(ANDROID_LOG_ERROR, "Urho3D", message.CString());
    }
    else
        __android_log_print(error ? ANDROID_LOG_ERROR : ANDROID_LOG_INFO, "Urho3D", message.CString());
    #elif defined(IOS)
    SDL_IOS_LogMessage(message.CString());
    #else
    if (logInstance->quiet_)
    {
        // If in quiet mode, still print the error message to the standard error stream
        if (error)
            PrintUnicode(message, true);
    }
    else
        PrintUnicode(message, error);
    #endif

    if (logInstance->logFile_)
    {
        logInstance->logFile_->Write(message.CString(), message.Length());
        logInstance->logFile_->Flush();
    }

    logInstance->inWrite_ = true;

    using namespace LogMessage;

    VariantMap& eventData = logInstance->GetEventDataMap();
    eventData[P_MESSAGE] = message;
    eventData[P_LEVEL] = error ? LOG_ERROR : LOG_INFO;
    logInstance->SendEvent(E_LOGMESSAGE, eventData);

    logInstance->inWrite_ = false;
}

}
