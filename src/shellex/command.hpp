/**
 * @file shellex/command.hpp
 *
 * @copyright 2018-2019 Bill Zissimopoulos
 */
/*
 * This file is part of WinSpd.
 *
 * You can redistribute it and/or modify it under the terms of the GNU
 * General Public License version 3 as published by the Free Software
 * Foundation.
 *
 * Licensees holding a valid commercial license may use this software
 * in accordance with the commercial license agreement provided in
 * conjunction with the software.  The terms and conditions of any such
 * commercial license agreement shall govern, supersede, and render
 * ineffective any application of the GPLv3 license to this software,
 * notwithstanding of any reference thereto in the software or
 * associated repository.
 */

#ifndef WINSPD_SHELLEX_COMMAND_HPP_INCLUDED
#define WINSPD_SHELLEX_COMMAND_HPP_INCLUDED

#include <shellex/com.hpp>
#include <shobjidl.h>

class Command : public CoObject<
    IInitializeCommand,
    IObjectWithSelection,
    IExecuteCommand,
    IExplorerCommandState>
{
public:
    /* IInitializeCommand */
    STDMETHODIMP Initialize(LPCWSTR CommandName, IPropertyBag *Bag)
    {
        return S_OK;
    }

    /* IObjectWithSelection */
    STDMETHODIMP SetSelection(IShellItemArray *Array)
    {
        if (0 != _Array)
            _Array->Release();
        if (0 != Array)
            Array->AddRef();
        _Array = Array;
        return S_OK;
    }
    STDMETHODIMP GetSelection(REFIID Iid, void **PObject)
    {
        if (0 != _Array)
            return _Array->QueryInterface(Iid, PObject);
        else
        {
            *PObject = 0;
            return E_NOINTERFACE;
        }
    }

    /* IExecuteCommand */
    STDMETHODIMP SetKeyState(DWORD KeyState)
    {
        return S_OK;
    }
    STDMETHODIMP SetParameters(LPCWSTR Parameters)
    {
        return S_OK;
    }
    STDMETHODIMP SetPosition(POINT Point)
    {
        return S_OK;
    }
    STDMETHODIMP SetShowWindow(int Show)
    {
        return S_OK;
    }
    STDMETHODIMP SetNoShowUI(BOOL NoShowUI)
    {
        return S_OK;
    }
    STDMETHODIMP SetDirectory(LPCWSTR Directory)
    {
        return S_OK;
    }
    STDMETHODIMP Execute()
    {
        IEnumShellItems *Enum;
        IShellItem *Item;
        PWSTR Name;
        if (0 != _Array && S_OK == _Array->EnumItems(&Enum))
        {
            while (S_OK == Enum->Next(1, &Item, 0))
            {
                if (S_OK == Item->GetDisplayName(SIGDN_FILESYSPATH, &Name))
                {
                    OutputDebugStringA(__func__);
                    OutputDebugStringA(" ");
                    OutputDebugStringW(Name);
                    OutputDebugStringA("\n");
                    CoTaskMemFree(Name);
                }
                Item->Release();
            }
            Enum->Release();
        }
        return S_OK;
    }

    /* IExplorerCommandState */
    STDMETHODIMP GetState(IShellItemArray *Array, BOOL OkToBeSlow, EXPCMDSTATE *CmdState)
    {
        *CmdState = ECS_ENABLED;
        return S_OK;
    }

    /* internal interface */
    static STDMETHODIMP RegisterEx(BOOL Flag,
        PWSTR Progid, PWSTR Verb, PWSTR VerbDesc, BOOL CommandStateHandler,
        REFCLSID Clsid)
    {
        HKEY ClassesKey, ProgKey, ShellKey, VerbKey, CommandKey;
        WCHAR GuidStr[40];
        HRESULT Result;
        ClassesKey = ProgKey = ShellKey = VerbKey = CommandKey = 0;
        Result = HRESULT_FROM_WIN32(RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes",
            0, KEY_ALL_ACCESS, &ClassesKey));
        if (S_OK != Result)
            goto exit;
        if (Flag)
        {
            Result = HRESULT_FROM_WIN32(RegCreateKeyExW(ClassesKey, Progid,
                0, 0, 0, KEY_ALL_ACCESS, 0, &ProgKey, 0));
            if (S_OK != Result)
                goto exit;
            Result = HRESULT_FROM_WIN32(RegCreateKeyExW(ProgKey, L"shell",
                0, 0, 0, KEY_ALL_ACCESS, 0, &ShellKey, 0));
            if (S_OK != Result)
                goto exit;
            Result = HRESULT_FROM_WIN32(RegCreateKeyExW(ShellKey, Verb,
                0, 0, 0, KEY_ALL_ACCESS, 0, &VerbKey, 0));
            if (S_OK != Result)
                goto exit;
            Result = HRESULT_FROM_WIN32(RegCreateKeyExW(VerbKey, L"command",
                0, 0, 0, KEY_ALL_ACCESS, 0, &CommandKey, 0));
            if (S_OK != Result)
                goto exit;
            Result = HRESULT_FROM_WIN32(RegSetValueExW(VerbKey,
                0, 0, REG_SZ,
                (BYTE *)VerbDesc,
                (lstrlenW(VerbDesc) + 1) * sizeof(WCHAR)));
            if (S_OK != Result)
                goto exit;
            StringFromGUID2(Clsid, GuidStr, sizeof GuidStr / sizeof GuidStr[0]);
            if (CommandStateHandler)
            {
                Result = HRESULT_FROM_WIN32(RegSetValueExW(VerbKey,
                    L"CommandStateHandler", 0, REG_SZ,
                    (BYTE *)GuidStr,
                    (lstrlenW(GuidStr) + 1) * sizeof(WCHAR)));
                if (S_OK != Result)
                    goto exit;
            }
            Result = HRESULT_FROM_WIN32(RegSetValueExW(CommandKey,
                L"DelegateExecute", 0, REG_SZ,
                (BYTE *)GuidStr,
                (lstrlenW(GuidStr) + 1) * sizeof(WCHAR)));
            if (S_OK != Result)
                goto exit;
        }
        else
            RegDeleteTreeW(ClassesKey, Progid);
    exit:
        if (0 != CommandKey)
            RegCloseKey(CommandKey);
        if (0 != VerbKey)
            RegCloseKey(VerbKey);
        if (0 != ShellKey)
            RegCloseKey(ShellKey);
        if (0 != ProgKey)
            RegCloseKey(ProgKey);
        if (0 != ClassesKey)
            RegCloseKey(ClassesKey);
        return Result;
    }
    Command() : _Array(0)
    {
    }
    ~Command()
    {
        if (0 != _Array)
            _Array->Release();
    }

private:
    IShellItemArray *_Array;
};

#endif
