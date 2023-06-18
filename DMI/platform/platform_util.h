 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
#include <memory>
#include <map>
#include <any>
#include <optional>

namespace PlatformUtil
{
	class NoCopy
	{
	public:
		NoCopy(const NoCopy&) = delete;
		NoCopy& operator=(const NoCopy&) = delete;
		NoCopy() = default;
	};

	template <typename T>
	struct CallbackType
	{
		CallbackType() = delete;
		typedef std::function<void(T&&)> type;
	};

	template <>
	struct CallbackType<void>
	{
		CallbackType() = delete;
		typedef std::function<void()> type;
	};

	template <typename T>
	struct TypeOrEmpty
	{
		TypeOrEmpty() = delete;
		typedef T type;
	};

	template <>
	struct TypeOrEmpty<void>
	{
		TypeOrEmpty() = delete;
		struct Empty {};
		typedef Empty type;
	};

	template <typename T>
	class Promise;
	template <typename T>
	class Fulfiller;
	class PromiseFactory;

	template <typename T>
	class PromiseStorage : private NoCopy
	{
		friend class Promise<T>;
		friend class Fulfiller<T>;

		typename CallbackType<T>::type callback;
		std::optional<typename TypeOrEmpty<T>::type> value;

		void subscribe(const typename CallbackType<T>::type &func) {
			callback = func;
			if (value) {
				if constexpr (std::is_void<T>::value)
					callback();
				else
					callback(std::move(*value));
			}
		}

		template <typename... Args>
		void fulfill(Args&&... args) {
			if constexpr (std::is_void<T>::value)
				value.emplace();
			else
				value = std::forward<T>(args...);

			if (callback) {
				if constexpr (std::is_void<T>::value)
					callback(args...);
				else
					callback(std::move(*value));
			}
		}
	};

	template <typename T>
	class Fulfiller
	{
		friend class PromiseFactory;

		std::shared_ptr<PromiseStorage<T>> storage;
		Fulfiller(const std::shared_ptr<PromiseStorage<T>> &ptr) : storage(ptr) {};

	public:
		Fulfiller() = default;
		Fulfiller(Fulfiller &&other) {
			storage = other.storage;
			other.storage = nullptr;
		}
		Fulfiller& operator=(Fulfiller &&other) {
			storage = other.storage;
			other.storage = nullptr;
			return *this;
		}

		bool is_pending() const {
			if (!storage)
				return false;
			return !storage->value;
		}

		template <typename... Args>
		void fulfill(Args&&... args) {
			if (!storage)
				return;

			if constexpr (std::is_void<T>::value)
				storage->fulfill(args...);
			else
				storage->fulfill(std::forward<T>(args...));

			storage = nullptr;
		}
	};

	template <typename T>
	class Promise
	{
		friend class PromiseFactory;

		std::shared_ptr<PromiseStorage<T>> storage;
		Promise(const std::shared_ptr<PromiseStorage<T>> &ptr) : storage(ptr) {};

	public:
		Promise() = default;
		Promise(Promise &&other) {
			storage = other.storage;
			other.storage = nullptr;
		}
		Promise& operator=(Promise &&other) {
			storage = other.storage;
			other.storage = nullptr;
			return *this;
		}

		void detach() {
			storage = nullptr;
		}

		bool is_pending() const {
			if (!storage)
				return false;
			return !storage->value;
		}

		~Promise() {
			if (storage)
				storage->callback = nullptr;
		}

		Promise<T>& then(const typename CallbackType<T>::type &func) {
			if (storage && !storage->callback)
				storage->subscribe(func);
			return *this;
		}
	};

	class PromiseFactory
	{
		PromiseFactory() = delete;

	public:
		template <typename T>
		static std::pair<Promise<T>, Fulfiller<T>> create() {
			std::shared_ptr<PromiseStorage<T>> storage = std::make_shared<PromiseStorage<T>>();
			return std::make_pair(Promise<T>(storage), Fulfiller<T>(storage));
		}
	};

	template <typename T>
	inline std::pair<Promise<T>, Fulfiller<T>> create_promise() {
		return PromiseFactory::create<T>();
	}
}
