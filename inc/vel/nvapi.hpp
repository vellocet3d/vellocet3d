#pragma once

#ifdef WINDOWS_BUILD

#pragma once

#include "nvapi/nvapi.h"
#include "nvapi/NvApiDriverSettings.h"
#include <string>

#include "vel/Log.h"

namespace vel
{
    bool nvapiStatusOk(NvAPI_Status status)
    {
        if (status != NVAPI_OK)
        {
#ifdef DEBUG_LOG
    // full list of codes in nvapi_lite_common.h line 249
    std::string msg = std::string("Status Code:") + std::to_string((int)status);
    Log::toCli(msg);
    Log::toFile(msg);
    
    NvAPI_ShortString szDesc = { 0 };
	NvAPI_GetErrorMessage(status, szDesc);
	//printf("NVAPI Error: %s\n", szDesc);
    
    msg = std::string(szDesc);
    Log::toCli(msg);
    Log::toFile(msg);
#endif

            return false;
        }

        return true;
    }


    void setNVUstring(NvAPI_UnicodeString& nvStr, const wchar_t* wcStr)
    {
        for (int i = 0; i < NVAPI_UNICODE_STRING_MAX; i++)
            nvStr[i] = 0;

        int i = 0;
        while (wcStr[i] != 0)
        {
            nvStr[i] = wcStr[i];
            i++;
        }
    }

    void initNvidiaApplicationProfile(std::string an, std::string pn)
    {
        std::wstring wan(an.begin(), an.end());
        const wchar_t*  appName = wan.c_str();

        std::wstring wpn(pn.begin(), pn.end());
        const wchar_t*  profileName = wpn.c_str();

        // could have a user defined struct containing more options if need be down the road
        const bool      threadedOptimization = false;


        NvAPI_Status status;

        // if status does not equal NVAPI_OK (0) after initialization,
        // either the system does not use an nvidia gpu, or something went
        // so wrong that we're unable to use the nvidia api...therefore do nothing
        // for debugging use ^ in prod
        if (!nvapiStatusOk(NvAPI_Initialize()))
        {
#ifdef DEBUG_LOG
    std::string msg = "Unable to initialize Nvidia api";
    Log::toCli(msg);
    Log::toFile(msg);
#endif

            return;
        }
#ifdef DEBUG_LOG
else
{
    std::string msg = "Nvidia api initialized successfully";
    Log::toCli(msg);
    Log::toFile(msg);
}
#endif
        
            
        // initialize session
        NvDRSSessionHandle hSession;
        if (!nvapiStatusOk(NvAPI_DRS_CreateSession(&hSession)))
            return;

        // load settings
        if (!nvapiStatusOk(NvAPI_DRS_LoadSettings(hSession)))
            return;

        // check if application already exists
        NvDRSProfileHandle hProfile;
        
        NvAPI_UnicodeString nvAppName;
        setNVUstring(nvAppName, appName);

        NVDRS_APPLICATION app;
        app.version = NVDRS_APPLICATION_VER_V1;

        // documentation states this will return ::NVAPI_APPLICATION_NOT_FOUND, however I cannot
        // find where that is defined anywhere in the headers...so not sure what's going to happen with this?
        //
        // This is returning NVAPI_EXECUTABLE_NOT_FOUND, which might be what it's supposed to return when it can't
        // find an existing application, and the documentation is just outdated?
        status = NvAPI_DRS_FindApplicationByName(hSession, nvAppName, &hProfile, &app);
        if (!nvapiStatusOk(status))
        {
            // if status does not equal NVAPI_EXECUTABLE_NOT_FOUND, then something bad happened and we should not proceed
            if (status != NVAPI_EXECUTABLE_NOT_FOUND)
            {
                NvAPI_Unload();
                return;
            }

            // create application as it does not already exist

            // Fill Profile Info
            NVDRS_PROFILE profileInfo;
            profileInfo.version = NVDRS_PROFILE_VER;
            profileInfo.isPredefined = 0;
            setNVUstring(profileInfo.profileName, profileName);

            // Create Profile
            //NvDRSProfileHandle hProfile;
            if (!nvapiStatusOk(NvAPI_DRS_CreateProfile(hSession, &profileInfo, &hProfile)))
            {
                NvAPI_Unload();
                return;
            }

            // Fill Application Info, can't re-use app variable for some reason
            NVDRS_APPLICATION app2;
            app2.version = NVDRS_APPLICATION_VER_V1;
            app2.isPredefined = 0;
            setNVUstring(app2.appName, appName);
            setNVUstring(app2.userFriendlyName, profileName);
            setNVUstring(app2.launcher, L"");
            setNVUstring(app2.fileInFolder, L"");

            // Create Application
            if (!nvapiStatusOk(NvAPI_DRS_CreateApplication(hSession, hProfile, &app2)))
            {
                NvAPI_Unload();
                return;
            }
        }

        // update profile settings
        NVDRS_SETTING setting;
        setting.version = NVDRS_SETTING_VER;
        setting.settingId = OGL_THREAD_CONTROL_ID;
        setting.settingType = NVDRS_DWORD_TYPE;
        setting.settingLocation = NVDRS_CURRENT_PROFILE_LOCATION;
        setting.isCurrentPredefined = 0;
        setting.isPredefinedValid = 0;
        setting.u32CurrentValue = threadedOptimization ? OGL_THREAD_CONTROL_ENABLE : OGL_THREAD_CONTROL_DISABLE;
        setting.u32PredefinedValue = threadedOptimization ? OGL_THREAD_CONTROL_ENABLE : OGL_THREAD_CONTROL_DISABLE;

        // load settings
        if (!nvapiStatusOk(NvAPI_DRS_SetSetting(hSession, hProfile, &setting)))
        {
            NvAPI_Unload();
            return;
        }

        // save changes
        if (!nvapiStatusOk(NvAPI_DRS_SaveSettings(hSession)))
        {
            NvAPI_Unload();
            return;
        }

#ifdef DEBUG_LOG
		std::string msg = "Nvidia application profile updated successfully";
		Log::toCli(msg);
		Log::toFile(msg);
#endif

        NvAPI_DRS_DestroySession(hSession);

        // unload the api as we're done with it
        NvAPI_Unload();
    }
}
#endif