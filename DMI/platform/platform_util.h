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

	class NoCopy
	{
	public:
		NoCopy(const NoCopy&) = delete;
		NoCopy& operator=(const NoCopy&) = delete;
		NoCopy() = default;
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
		std::optional<T> value;

		void subscribe(const typename CallbackType<T>::type &func) {
			callback = func;
			if (value)
				callback(std::move(*value));
		}

		void fulfill(T&& val) {
			value = std::move(val);
			if (callback)
				callback(std::move(*value));
		}

		void fulfill(const T& val) {
			value = val;
			if (callback)
				callback(std::move(*value));
		}
	};

	template <>
	class PromiseStorage<void> : private NoCopy
	{
		friend class Promise<void>;
		friend class Fulfiller<void>;

		typename CallbackType<void>::type callback;
		bool value = false;

		void subscribe(const typename CallbackType<void>::type &func) {
			callback = func;
			if (value)
				callback();
		}

		void fulfill() {
			value = true;
			if (callback)
				callback();
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

		void fulfill(T&& arg) {
			if (!storage)
				return;
			storage->fulfill(std::move(arg));
			storage = nullptr;
		}

		void fulfill(const T& arg) {
			if (!storage)
				return;
			storage->fulfill(arg);
			storage = nullptr;
		}
	};

	template <>
	class Fulfiller<void>
	{
		friend class PromiseFactory;

		std::shared_ptr<PromiseStorage<void>> storage;
		Fulfiller(const std::shared_ptr<PromiseStorage<void>> &ptr) : storage(ptr) {};

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

		void fulfill() {
			if (!storage)
				return;
			storage->fulfill();
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
	class FulfillerList
	{
		std::vector<Fulfiller<T>> list;

	public:
		void add(Fulfiller<T> &&f) {
			list.push_back(std::move(f));
		}

		void fulfill_one(const T& arg) {
			if (list.empty())
				return;
			std::vector<Fulfiller<T>> tmp = std::move(list);
			list.clear();
			tmp.begin()->fulfill(arg);
			list.insert(list.begin(), std::make_move_iterator(std::next(tmp.begin())), std::make_move_iterator(tmp.end()));
		}

		void fulfill_one(T&& arg) {
			if (list.empty())
				return;
			std::vector<Fulfiller<T>> tmp = std::move(list);
			list.clear();
			tmp.begin()->fulfill(std::move(arg));
			list.insert(list.begin(), std::make_move_iterator(std::next(tmp.begin())), std::make_move_iterator(tmp.end()));
		}

		void fulfill_all(const T& arg) {
			std::vector<Fulfiller<T>> tmp = std::move(list);
			list.clear();
			for (PlatformUtil::Fulfiller<T> &f : tmp)
				f.fulfill(arg);
		}

		Promise<T> create_and_add() {
			std::pair<Promise<T>, Fulfiller<T>> pair = PromiseFactory::create<T>();
			list.push_back(std::move(pair.second));
			return std::move(pair.first);
		}
	};

	template <>
	class FulfillerList<void>
	{
		std::vector<Fulfiller<void>> list;

	public:
		void add(Fulfiller<void> &&f) {
			list.push_back(std::move(f));
		}

		size_t pending() {
			return list.size();
		}

		void fulfill_one() {
			if (list.empty())
				return;
			std::vector<Fulfiller<void>> tmp = std::move(list);
			list.clear();
			tmp.begin()->fulfill();
			list.insert(list.begin(), std::make_move_iterator(std::next(tmp.begin())), std::make_move_iterator(tmp.end()));
		}

		void fulfill_all() {
			std::vector<Fulfiller<void>> tmp = std::move(list);
			list.clear();
			for (PlatformUtil::Fulfiller<void> &f : tmp)
				f.fulfill();
		}

		Promise<void> create_and_add() {
			std::pair<Promise<void>, Fulfiller<void>> pair = PromiseFactory::create<void>();
			list.push_back(std::move(pair.second));
			return std::move(pair.first);
		}
	};
}
