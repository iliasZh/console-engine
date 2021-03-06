#pragma once

#include "windows.hpp"

#include <algorithm>
#include <cassert>
#include <string_view>

class FontInfoEx
{
public:
	/// Height to width ratio is always exactly 2. Clamps height to a certain
	/// range. If height == 2n + 1 is given, sets font height to 2n.
	// Intentionally not explicit.
	[[maybe_unused]] constexpr FontInfoEx(
		const int				height,
		const std::wstring_view face_name = default_font) noexcept
	{
		auto h = std::clamp(height - height % 2, min_height, max_height);
		assert(h % 2 == 0); // must be even
		auto w = h / 2;

		m_info.cbSize	  = sizeof(m_info);
		m_info.nFont	  = 0;
		m_info.dwFontSize = { static_cast<SHORT>(w), static_cast<SHORT>(h) };
		m_info.FontFamily = FF_DONTCARE;
		m_info.FontWeight = FW_NORMAL;

		face_name.copy(static_cast<wchar_t*>(m_info.FaceName),
					   std::size(m_info.FaceName));
	}

	explicit constexpr FontInfoEx(const CONSOLE_FONT_INFOEX info) noexcept
		: m_info{ info }
	{
		// if forgot to set cbSize to sizeof(m_info)
		assert(m_info.cbSize != 0);
	}

	[[nodiscard, maybe_unused]] constexpr const auto& cref() const noexcept
	{
		return m_info;
	}

	[[nodiscard]] constexpr auto& ref() noexcept
	{
		return m_info;
	}

	// These are arbitrary - I chose personally comfortable values.
	static constexpr int min_height = 14;
	static constexpr int max_height = 72;

	static constexpr auto default_font = L"Consolas";
private:
	CONSOLE_FONT_INFOEX m_info = {};
};
