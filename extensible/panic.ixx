module;

#include <stdexcept>
#include <utility>
#include <string>
#include <string_view>
#include <format>

export module Mitama.Data.Panic;

namespace mitama {

	export class runtime_panic: public std::runtime_error
	{
	public:
		template <class ...Args>
		runtime_panic(std::string_view fmt, Args&&... args)
			: runtime_error{ std::format(fmt, std::forward<Args>(args)...) }
		{}

		~runtime_panic() = default;
	};


} //! namespace mitama
