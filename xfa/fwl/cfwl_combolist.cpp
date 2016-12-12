// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_combolist.h"

#include <memory>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_combobox.h"
#include "xfa/fwl/cfwl_comboedit.h"
#include "xfa/fwl/cfwl_listbox.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagekillfocus.h"
#include "xfa/fwl/cfwl_messagemouse.h"

CFWL_ComboList::CFWL_ComboList(
    const CFWL_App* app,
    std::unique_ptr<CFWL_WidgetProperties> properties,
    CFWL_Widget* pOuter)
    : CFWL_ListBox(app, std::move(properties), pOuter), m_bNotifyOwner(true) {
  ASSERT(pOuter);
}

int32_t CFWL_ComboList::MatchItem(const CFX_WideString& wsMatch) {
  if (wsMatch.IsEmpty())
    return -1;

  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListItem* hItem = GetItem(this, i);
    CFX_WideString wsText = GetItemText(this, hItem);
    FX_STRSIZE pos = wsText.Find(wsMatch.c_str());
    if (!pos)
      return i;
  }
  return -1;
}

void CFWL_ComboList::ChangeSelected(int32_t iSel) {
  CFWL_ListItem* hItem = GetItem(this, iSel);
  CFX_RectF rtInvalidate;
  rtInvalidate.Reset();
  CFWL_ListItem* hOld = GetSelItem(0);
  int32_t iOld = GetItemIndex(this, hOld);
  if (iOld == iSel)
    return;
  if (iOld > -1) {
    CFWL_ListItem* hItem = GetItem(this, iOld);
    GetItemRect(this, hItem, rtInvalidate);
    SetSelItem(hOld, false);
  }
  if (hItem) {
    CFX_RectF rect;
    CFWL_ListItem* hItem = GetItem(this, iSel);
    GetItemRect(this, hItem, rect);
    rtInvalidate.Union(rect);
    CFWL_ListItem* hSel = GetItem(this, iSel);
    SetSelItem(hSel, true);
  }
  if (!rtInvalidate.IsEmpty())
    RepaintRect(rtInvalidate);
}

void CFWL_ComboList::ClientToOuter(FX_FLOAT& fx, FX_FLOAT& fy) {
  fx += m_pProperties->m_rtWidget.left, fy += m_pProperties->m_rtWidget.top;
  CFWL_Widget* pOwner = GetOwner();
  if (!pOwner)
    return;
  pOwner->TransformTo(m_pOuter, fx, fy);
}

void CFWL_ComboList::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  CFWL_Message::Type type = pMessage->GetType();
  bool backDefault = true;
  if (type == CFWL_Message::Type::SetFocus ||
      type == CFWL_Message::Type::KillFocus) {
    OnDropListFocusChanged(pMessage, type == CFWL_Message::Type::SetFocus);
  } else if (type == CFWL_Message::Type::Mouse) {
    CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
    CFWL_ScrollBar* vertSB = GetVertScrollBar();
    if (IsShowScrollBar(true) && vertSB) {
      CFX_RectF rect = vertSB->GetWidgetRect();
      if (rect.Contains(pMsg->m_fx, pMsg->m_fy)) {
        pMsg->m_fx -= rect.left;
        pMsg->m_fy -= rect.top;
        vertSB->GetDelegate()->OnProcessMessage(pMsg);
        return;
      }
    }
    switch (pMsg->m_dwCmd) {
      case FWL_MouseCommand::Move: {
        backDefault = false;
        OnDropListMouseMove(pMsg);
        break;
      }
      case FWL_MouseCommand::LeftButtonDown: {
        backDefault = false;
        OnDropListLButtonDown(pMsg);
        break;
      }
      case FWL_MouseCommand::LeftButtonUp: {
        backDefault = false;
        OnDropListLButtonUp(pMsg);
        break;
      }
      default:
        break;
    }
  } else if (type == CFWL_Message::Type::Key) {
    backDefault = !OnDropListKey(static_cast<CFWL_MessageKey*>(pMessage));
  }
  if (backDefault)
    CFWL_ListBox::OnProcessMessage(pMessage);
}

void CFWL_ComboList::OnDropListFocusChanged(CFWL_Message* pMsg, bool bSet) {
  if (bSet)
    return;

  CFWL_MessageKillFocus* pKill = static_cast<CFWL_MessageKillFocus*>(pMsg);
  CFWL_ComboBox* pOuter = static_cast<CFWL_ComboBox*>(m_pOuter);
  if (pKill->m_pSetFocus == m_pOuter ||
      pKill->m_pSetFocus == pOuter->GetComboEdit()) {
    pOuter->ShowDropList(false);
  }
}

void CFWL_ComboList::OnDropListMouseMove(CFWL_MessageMouse* pMsg) {
  if (GetRTClient().Contains(pMsg->m_fx, pMsg->m_fy)) {
    if (m_bNotifyOwner)
      m_bNotifyOwner = false;

    CFWL_ScrollBar* vertSB = GetVertScrollBar();
    if (IsShowScrollBar(true) && vertSB) {
      CFX_RectF rect = vertSB->GetWidgetRect();
      if (rect.Contains(pMsg->m_fx, pMsg->m_fy))
        return;
    }

    CFWL_ListItem* hItem = GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
    if (!hItem)
      return;

    ChangeSelected(GetItemIndex(this, hItem));
  } else if (m_bNotifyOwner) {
    ClientToOuter(pMsg->m_fx, pMsg->m_fy);
    CFWL_ComboBox* pOuter = static_cast<CFWL_ComboBox*>(m_pOuter);
    pOuter->GetDelegate()->OnProcessMessage(pMsg);
  }
}

void CFWL_ComboList::OnDropListLButtonDown(CFWL_MessageMouse* pMsg) {
  if (GetRTClient().Contains(pMsg->m_fx, pMsg->m_fy))
    return;

  CFWL_ComboBox* pOuter = static_cast<CFWL_ComboBox*>(m_pOuter);
  pOuter->ShowDropList(false);
}

void CFWL_ComboList::OnDropListLButtonUp(CFWL_MessageMouse* pMsg) {
  CFWL_ComboBox* pOuter = static_cast<CFWL_ComboBox*>(m_pOuter);
  if (m_bNotifyOwner) {
    ClientToOuter(pMsg->m_fx, pMsg->m_fy);
    pOuter->GetDelegate()->OnProcessMessage(pMsg);
    return;
  }

  CFWL_ScrollBar* vertSB = GetVertScrollBar();
  if (IsShowScrollBar(true) && vertSB) {
    CFX_RectF rect = vertSB->GetWidgetRect();
    if (rect.Contains(pMsg->m_fx, pMsg->m_fy))
      return;
  }
  pOuter->ShowDropList(false);

  CFWL_ListItem* hItem = GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
  if (hItem)
    pOuter->ProcessSelChanged(true);
}

bool CFWL_ComboList::OnDropListKey(CFWL_MessageKey* pKey) {
  CFWL_ComboBox* pOuter = static_cast<CFWL_ComboBox*>(m_pOuter);
  bool bPropagate = false;
  if (pKey->m_dwCmd == FWL_KeyCommand::KeyDown) {
    uint32_t dwKeyCode = pKey->m_dwKeyCode;
    switch (dwKeyCode) {
      case FWL_VKEY_Return:
      case FWL_VKEY_Escape: {
        pOuter->ShowDropList(false);
        return true;
      }
      case FWL_VKEY_Up:
      case FWL_VKEY_Down: {
        OnDropListKeyDown(pKey);
        pOuter->ProcessSelChanged(false);
        return true;
      }
      default: {
        bPropagate = true;
        break;
      }
    }
  } else if (pKey->m_dwCmd == FWL_KeyCommand::Char) {
    bPropagate = true;
  }
  if (bPropagate) {
    pKey->m_pDstTarget = m_pOuter;
    pOuter->GetDelegate()->OnProcessMessage(pKey);
    return true;
  }
  return false;
}

void CFWL_ComboList::OnDropListKeyDown(CFWL_MessageKey* pKey) {
  uint32_t dwKeyCode = pKey->m_dwKeyCode;
  switch (dwKeyCode) {
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_End: {
      CFWL_ComboBox* pOuter = static_cast<CFWL_ComboBox*>(m_pOuter);
      CFWL_ListItem* hItem = GetItem(this, pOuter->GetCurrentSelection());
      hItem = GetListItem(hItem, dwKeyCode);
      if (!hItem)
        break;

      SetSelection(hItem, hItem, true);
      ScrollToVisible(hItem);
      CFX_RectF rtInvalidate;
      rtInvalidate.Set(0, 0, m_pProperties->m_rtWidget.width,
                       m_pProperties->m_rtWidget.height);
      RepaintRect(rtInvalidate);
      break;
    }
    default:
      break;
  }
}