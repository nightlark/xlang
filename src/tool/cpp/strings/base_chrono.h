
namespace winrt::impl
{
    using filetime_period = std::ratio_multiply<std::ratio<100>, std::nano>;
}

WINRT_EXPORT namespace winrt
{
    struct clock;

    namespace Windows::Foundation
    {
        using TimeSpan = std::chrono::duration<int64_t, impl::filetime_period>;
        using DateTime = std::chrono::time_point<clock, TimeSpan>;
    }
}

namespace winrt::impl
{
    template <> struct abi<Windows::Foundation::TimeSpan>
    {
        using type = int64_t;
    };

    template <> struct abi<Windows::Foundation::DateTime>
    {
        using type = int64_t;
    };

    template <> struct name<Windows::Foundation::TimeSpan>
    {
        static constexpr auto & value{ L"Windows.Foundation.TimeSpan" };
    };

    template <> struct category<Windows::Foundation::TimeSpan>
    {
        using type = struct_category<int64_t>;
    };

    template <> struct name<Windows::Foundation::DateTime>
    {
        static constexpr auto & value{ L"Windows.Foundation.DateTime" };
    };

    template <> struct category<Windows::Foundation::DateTime>
    {
        using type = struct_category<int64_t>;
    };
}

WINRT_EXPORT namespace winrt
{
    struct file_time
    {
        uint64_t value{};

        file_time() noexcept = default;

        constexpr explicit file_time(uint64_t const value) noexcept : value(value)
        {
        }

#ifdef _FILETIME_
        constexpr file_time(FILETIME const& value) noexcept
            : value(value.dwLowDateTime | (static_cast<uint64_t>(value.dwHighDateTime) << 32))
        {
        }

        operator FILETIME() const noexcept
        {
            return { static_cast<DWORD>(value & 0xFFFFFFFF), static_cast<DWORD>(value >> 32) };
        }
#endif
    };

    struct clock
    {
        using rep = int64_t;
        using period = impl::filetime_period;
        using duration = Windows::Foundation::TimeSpan;
        using time_point = Windows::Foundation::DateTime;

        static constexpr bool is_steady = false;

        static time_point now() noexcept
        {
            file_time ft;
            WINRT_GetSystemTimePreciseAsFileTime(&ft);
            return from_file_time(ft);
        }

        static time_t to_time_t(time_point const& time) noexcept
        {
            return std::chrono::duration_cast<time_t_duration>(time - time_t_epoch).count();
        }

        static time_point from_time_t(time_t time) noexcept
        {
            return time_t_epoch + time_t_duration{ time };
        }

        static file_time to_file_time(time_point const& time) noexcept
        {
            return file_time{ static_cast<uint64_t>(time.time_since_epoch().count()) };
        }

        static time_point from_file_time(file_time const& time) noexcept
        {
            return time_point{ duration{ time.value } };
        }

        static auto to_FILETIME(time_point const& time) noexcept
        {
            return to_file_time(time);
        }

        static time_point from_FILETIME(file_time const& time) noexcept
        {
            return from_file_time(time);
        }

    private:

        // Define 00:00:00, Jan 1 1970 UTC in FILETIME units.
        static constexpr time_point time_t_epoch{ duration{ 0x019DB1DED53E8000 } };
        using time_t_duration = std::chrono::duration<time_t>;
    };
}
