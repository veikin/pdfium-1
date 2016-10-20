// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_BARCODE_H_
#define XFA_FWL_CORE_IFWL_BARCODE_H_

#include <memory>

#include "xfa/fwl/core/ifwl_edit.h"
#include "xfa/fwl/core/ifwl_scrollbar.h"
#include "xfa/fxbarcode/BC_Library.h"

class CFWL_BarcodeImpDelegate;
class CFWL_WidgetImpProperties;
class CFX_Barcode;
class IFWL_Widget;

#define XFA_BCS_NeedUpdate 0x0001
#define XFA_BCS_EncodeSuccess 0x0002
#define FWL_CLASS_Barcode L"FWL_BARCODE"

enum FWL_BCDAttribute {
  FWL_BCDATTRIBUTE_NONE = 0,
  FWL_BCDATTRIBUTE_CHARENCODING = 1 << 0,
  FWL_BCDATTRIBUTE_MODULEHEIGHT = 1 << 1,
  FWL_BCDATTRIBUTE_MODULEWIDTH = 1 << 2,
  FWL_BCDATTRIBUTE_DATALENGTH = 1 << 3,
  FWL_BCDATTRIBUTE_CALCHECKSUM = 1 << 4,
  FWL_BCDATTRIBUTE_PRINTCHECKSUM = 1 << 5,
  FWL_BCDATTRIBUTE_TEXTLOCATION = 1 << 6,
  FWL_BCDATTRIBUTE_WIDENARROWRATIO = 1 << 7,
  FWL_BCDATTRIBUTE_STARTCHAR = 1 << 8,
  FWL_BCDATTRIBUTE_ENDCHAR = 1 << 9,
  FWL_BCDATTRIBUTE_VERSION = 1 << 10,
  FWL_BCDATTRIBUTE_ECLEVEL = 1 << 11,
  FWL_BCDATTRIBUTE_TRUNCATED = 1 << 12
};

class IFWL_BarcodeDP : public IFWL_EditDP {
 public:
  virtual BC_CHAR_ENCODING GetCharEncoding() = 0;
  virtual int32_t GetModuleHeight() = 0;
  virtual int32_t GetModuleWidth() = 0;
  virtual int32_t GetDataLength() = 0;
  virtual int32_t GetCalChecksum() = 0;
  virtual FX_BOOL GetPrintChecksum() = 0;
  virtual BC_TEXT_LOC GetTextLocation() = 0;
  virtual int32_t GetWideNarrowRatio() = 0;
  virtual FX_CHAR GetStartChar() = 0;
  virtual FX_CHAR GetEndChar() = 0;
  virtual int32_t GetVersion() = 0;
  virtual int32_t GetErrorCorrectionLevel() = 0;
  virtual FX_BOOL GetTruncated() = 0;
  virtual uint32_t GetBarcodeAttributeMask() = 0;
};

class IFWL_Barcode : public IFWL_Edit {
 public:
  static IFWL_Barcode* Create(const CFWL_WidgetImpProperties& properties);

  IFWL_Barcode(const CFWL_WidgetImpProperties& properties, IFWL_Widget* pOuter);
  ~IFWL_Barcode() override;

  // IFWL_Widget
  FWL_Error GetClassName(CFX_WideString& wsClass) const override;
  FWL_Type GetClassID() const override;
  FWL_Error Initialize() override;
  FWL_Error Finalize() override;
  FWL_Error Update() override;
  FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr) override;
  FWL_Error SetText(const CFX_WideString& wsText) override;

  void SetType(BC_TYPE type);
  FX_BOOL IsProtectedType();

 protected:
  friend class CFWL_BarcodeImpDelegate;

  void GenerateBarcodeImageCache();
  void CreateBarcodeEngine();

  std::unique_ptr<CFX_Barcode> m_pBarcodeEngine;
  uint32_t m_dwStatus;
  BC_TYPE m_type;
};

class CFWL_BarcodeImpDelegate : public CFWL_EditImpDelegate {
 public:
  explicit CFWL_BarcodeImpDelegate(IFWL_Barcode* pOwner);
  void OnProcessEvent(CFWL_Event* pEvent) override;
};

#endif  // XFA_FWL_CORE_IFWL_BARCODE_H_