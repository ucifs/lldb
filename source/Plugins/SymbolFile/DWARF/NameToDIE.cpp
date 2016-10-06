//===-- NameToDIE.cpp -------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NameToDIE.h"
#include "lldb/Core/ConstString.h"
#include "lldb/Core/RegularExpression.h"
#include "lldb/Core/Stream.h"
#include "lldb/Core/StreamString.h"
#include "lldb/Symbol/ObjectFile.h"

#include "DWARFCompileUnit.h"
#include "DWARFDebugInfo.h"
#include "DWARFDebugInfoEntry.h"
#include "SymbolFileDWARF.h"

using namespace lldb;
using namespace lldb_private;

void NameToDIE::Finalize() {
  m_map.Sort();
  m_map.SizeToFit();
}

void NameToDIE::Insert(const ConstString &name, const DIERef &die_ref) {
  m_map.Append(name.GetStringRef(), die_ref);
}

size_t NameToDIE::Find(const ConstString &name, DIEArray &info_array) const {
  return m_map.GetValues(name.GetStringRef(), info_array);
}

size_t NameToDIE::Find(const RegularExpression &regex,
                       DIEArray &info_array) const {
  return m_map.GetValues(regex, info_array);
}

size_t NameToDIE::FindAllEntriesForCompileUnit(dw_offset_t cu_offset,
                                               DIEArray &info_array) const {
  const size_t initial_size = info_array.size();
  const uint32_t size = m_map.GetSize();
  for (uint32_t i = 0; i < size; ++i) {
    const DIERef &die_ref = m_map.GetValueAtIndexUnchecked(i);
    if (cu_offset == die_ref.cu_offset)
      info_array.push_back(die_ref);
  }
  return info_array.size() - initial_size;
}

void NameToDIE::Dump(Stream *s) {
  const uint32_t size = m_map.GetSize();
  for (uint32_t i = 0; i < size; ++i) {
    llvm::StringRef cstr = m_map.GetCStringAtIndex(i);
    const DIERef &die_ref = m_map.GetValueAtIndexUnchecked(i);
    s->Printf("%p: {0x%8.8x/0x%8.8x} \"%s\"\n", (const void *)cstr.data(),
              die_ref.cu_offset, die_ref.die_offset, cstr);
  }
}

void NameToDIE::ForEach(
    std::function<bool(llvm::StringRef name, const DIERef &die_ref)> const
        &callback) const {
  const uint32_t size = m_map.GetSize();
  for (uint32_t i = 0; i < size; ++i) {
    if (!callback(m_map.GetCStringAtIndexUnchecked(i),
                  m_map.GetValueAtIndexUnchecked(i)))
      break;
  }
}

void NameToDIE::Append(const NameToDIE &other) {
  const uint32_t size = other.m_map.GetSize();
  for (uint32_t i = 0; i < size; ++i) {
    m_map.Append(other.m_map.GetCStringAtIndexUnchecked(i),
                 other.m_map.GetValueAtIndexUnchecked(i));
  }
}
