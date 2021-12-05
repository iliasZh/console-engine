#pragma once

#include "windows.hpp"

#include <algorithm>
#include <string>

class CursorInfo
{
public:
	constexpr CursorInfo(CONSOLE_CURSOR_INFO info) noexcept
		: m_info{ clamp_size(info.dwSize), info.bVisible }
	{}

	constexpr CursorInfo(const int size, const bool visible) noexcept
		: CursorInfo{ CONSOLE_CURSOR_INFO{ static_cast<DWORD>(size), static_cast<BOOL>(visible) } }
	{}

	[[nodiscard]] constexpr const auto& cref() const noexcept
	{
		return m_info;
	}

	[[nodiscard]] constexpr auto& ref() noexcept
	{
		return m_info;
	}

	// See MSDN on why these values.
	static constexpr int min_size = 1;
	static constexpr int max_size = 100;
private:
	static constexpr DWORD clamp_size(const DWORD size)
	{
		return std::clamp(size, static_cast<DWORD>(min_size), static_cast<DWORD>(max_size));
	}

	CONSOLE_CURSOR_INFO m_info = { static_cast<DWORD>(min_size), static_cast<BOOL>(true) };
};