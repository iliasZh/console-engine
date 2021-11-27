#include "console.hpp"
#include "exception.hpp"
#include "win_utils.hpp"

#include <array>
#include <consoleapi2.h>
#include <format>

namespace console
{
constexpr int time_to_apply_changes_ms = 50;

void set_cursor_info(const CursorInfo cursor_info)
{
	THROW_IF_ZERO(SetConsoleCursorInfo(win_utils::std_out(), &cursor_info.cref()),
				  L"failed to set console cursor");
}

[[nodiscard]] CursorInfo get_cursor_info()
{
	CONSOLE_CURSOR_INFO info{};

	THROW_IF_ZERO(GetConsoleCursorInfo(win_utils::std_out(), &info),
				  L"failed to get console cursor info");

	return CursorInfo{ info };
}

void set_font_info_ex(FontInfoEx font_info)
{
	THROW_IF_ZERO(SetCurrentConsoleFontEx(win_utils::std_out(), FALSE, &font_info.ref()),
				  L"failed to set console font");

	Sleep(time_to_apply_changes_ms);
}

void set_screen_buffer_size(const ScreenBufferSize screen_buffer_size)
{
	auto set_console_window_size = [](const COORD dims) {
		SMALL_RECT window;
		window.Left	  = 0;
		window.Top	  = 0;
		window.Right  = static_cast<SHORT>(dims.X - 1);
		window.Bottom = static_cast<SHORT>(dims.Y - 1);

		THROW_IF_ZERO(SetConsoleWindowInfo(win_utils::std_out(), TRUE, &window),
					  std::format(L"failed to set console window size to {}*{}", dims.X, dims.Y));
	};

	// At any time console window size must not exceed the size of console screen buffer.
	// For that reason set window size to minimum so that it is possible to set screen
	// buffer size without a problem.
	set_console_window_size({ 1, 1 });

	THROW_IF_ZERO(SetConsoleScreenBufferSize(win_utils::std_out(), screen_buffer_size.get()),
				  L"failed to set requested console screen buffer size");

	set_console_window_size(screen_buffer_size.get());
}

/// It takes some tens of milliseconds to set the title.
void set_title(const std::wstring_view title)
{
	THROW_IF_ZERO(SetConsoleTitleW(title.data()),
				  std::format(L"failed to set console title to \"{}\"", title));

	Sleep(time_to_apply_changes_ms);
}

[[nodiscard]] DWORD get_mode(HANDLE h_buf)
{
	// The warning is about the handle macro using C-style cast - can't do anything about it.
	assert(h_buf != INVALID_HANDLE_VALUE); // NOLINT

	DWORD mode = 0;

	THROW_IF_ZERO(GetConsoleMode(h_buf, &mode), L"failed to get console's current i/o mode");

	return mode;
}

void set_mode(HANDLE h_buf, const DWORD mode)
{
	THROW_IF_ZERO(SetConsoleMode(h_buf, mode), L"failed to set console mode");
}

void set_quick_edit_mode(const bool enable)
{
	HANDLE std_in = win_utils::std_in();

	const auto current_mode = get_mode(std_in);

	const bool selection_enabled = (current_mode & ENABLE_QUICK_EDIT_MODE) != 0;

	if (selection_enabled != enable) {
		set_mode(std_in, ENABLE_EXTENDED_FLAGS | (current_mode ^ ENABLE_QUICK_EDIT_MODE));
	}
}

[[nodiscard]] bool get_quick_edit_mode()
{
	HANDLE std_in = win_utils::std_in();

	return (get_mode(std_in) & ENABLE_QUICK_EDIT_MODE) != 0;
}

/// Maximum of 256 characters. Truncates if the title is bigger.
[[nodiscard]] std::wstring get_title()
{
	constexpr int buf_size = 256;

	std::array<wchar_t, buf_size> title_buf{};

	const auto chars_written = GetConsoleTitleW(title_buf.data(), buf_size);
	if (chars_written == 0) {
		THROW_EXCEPTION(L"failed to get console window title");
	}

	return { title_buf.cbegin(), std::next(title_buf.cbegin(), chars_written) };
}

[[nodiscard]] HWND get_handle_to_window()
{
	auto current_title = get_title();

	// Change the title to avoid (unlikely, but) possible collisions.
	std::wstring_view temp_title = L"console engine by Ilias, this is a temporary title";
	set_title(temp_title);

	HWND handle = FindWindowW(nullptr, temp_title.data());
	if (handle == nullptr) {
		THROW_EXCEPTION(std::format(L"failed to find window named \"{}\"", temp_title));
	}

	set_title(current_title);

	return handle;
}
} // namespace console
