/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace slib
{
	
	struct _priv_String16_Const
	{
		StringContainer16* container;
		sl_int32 lock;
	};
	
	extern const _priv_String16_Const _priv_String16_Null;
	extern const _priv_String16_Const _priv_String16_Empty;

	template <>
	struct StringTypeFromCharType<sl_char16> { typedef String16 Type; };

	SLIB_INLINE String16::String16(String16&& src)
	{
		m_container = src.m_container;
		src.m_container = sl_null;
	}

	SLIB_INLINE String16::String16(AtomicString16&& src)
	{
		m_container = src.m_container;
		src.m_container = sl_null;
	}
	
	SLIB_INLINE const String16& String16::null()
	{
		return *(reinterpret_cast<String16 const*>(&_priv_String16_Null));
	}
	
	SLIB_INLINE const String16& String16::getEmpty()
	{
		return *(reinterpret_cast<String16 const*>(&_priv_String16_Empty));
	}

	SLIB_INLINE const String16& String16::getNotNull() const
	{
		if (!m_container) {
			return *(reinterpret_cast<String16 const*>(&_priv_String16_Empty));
		}
		return *this;
	}
	
	SLIB_INLINE sl_bool String16::isNull() const
	{
		return m_container == sl_null;
	}
	
	SLIB_INLINE sl_bool String16::isNotNull() const
	{
		return m_container != sl_null;
	}
	
	SLIB_INLINE sl_bool String16::isEmpty() const
	{
		if (m_container) {
			return (m_container->len == 0);
		} else {
			return sl_true;
		}
	}
	
	SLIB_INLINE sl_bool String16::isNotEmpty() const
	{
		if (m_container) {
			return (m_container->len != 0);
		} else {
			return sl_false;
		}
	}
	
	SLIB_INLINE sl_char16* String16::getData() const
	{
		if (m_container) {
			return m_container->sz;
		} else {
			return (sl_char16*)((void*)(L""));
		}
	}
	
	SLIB_INLINE sl_size String16::getLength() const
	{
		if (m_container) {
			return m_container->len;
		} else {
			return 0;
		}
	}


	SLIB_INLINE AtomicString16::Atomic(AtomicString16&& src)
	{
		m_container = src.m_container;
		src.m_container = sl_null;
	}
	
	SLIB_INLINE AtomicString16::Atomic(String16&& src)
	{
		m_container = src.m_container;
		src.m_container = sl_null;
	}

	SLIB_INLINE const AtomicString16& AtomicString16::null()
	{
		return *(reinterpret_cast<AtomicString16 const*>(&_priv_String16_Null));
	}
	
	SLIB_INLINE const AtomicString16& AtomicString16::getEmpty()
	{
		return *(reinterpret_cast<AtomicString16 const*>(&_priv_String16_Empty));
	}
	
	SLIB_INLINE sl_bool AtomicString16::isNull() const
	{
		return m_container == sl_null;
	}
	
	SLIB_INLINE sl_bool AtomicString16::isNotNull() const
	{
		return m_container != sl_null;
	}

}
