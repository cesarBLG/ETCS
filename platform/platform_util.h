 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
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
	class PromisePart
	{
		friend class PromiseFactory;
		friend class Promise<T>;
		friend class Fulfiller<T>;

		Fulfiller<T>* fulfiller;
	};

	template <typename T>
	class Fulfiller
	{
		friend class PromiseFactory;
		friend class Promise<T>;

		PromisePart<T>* promise;
		std::optional<T> value;
		typename CallbackType<T>::type callback;
		bool allocated;

	public:
		Fulfiller() {
			promise = nullptr;
			allocated = false;
		}
		Fulfiller(Fulfiller &&other) {
			promise = nullptr;
			allocated = false;
			*this = std::move(other);
		}
		Fulfiller& operator=(Fulfiller &&other) {
			if (promise)
				promise->fulfiller = nullptr;

			promise = other.promise;
			if (promise)
				promise->fulfiller = this;

			value = std::move(other.value);
			callback = std::move(other.callback);

			other.promise = nullptr;
			other.callback = nullptr;

			return *this;
		}
		~Fulfiller() {
			if (promise) {
				if (value)
					(new Fulfiller<T>(std::move(*this)))->allocated = true;
				else
					promise->fulfiller = nullptr;
			}
		}

		bool is_pending() const {
			if (value)
				return false;
			return callback != nullptr || promise != nullptr;
		}

		void fulfill(T&& arg) {
			value = std::move(arg);
			if (callback) {
				callback(std::move(*value));
				callback = nullptr;
			}
		}

		void fulfill(const T& arg) {
			value = arg;
			if (callback) {
				callback(std::move(*value));
				callback = nullptr;
			}
		}
	};

	template <>
	class Fulfiller<void>
	{
		friend class PromiseFactory;
		friend class Promise<void>;

		PromisePart<void>* promise;
		bool value;
		typename CallbackType<void>::type callback;
		bool allocated;

	public:
		Fulfiller() {
			promise = nullptr;
			value = false;
			allocated = false;
		}
		Fulfiller(Fulfiller &&other) {
			promise = nullptr;
			allocated = false;
			*this = std::move(other);
		}
		Fulfiller& operator=(Fulfiller &&other) {
			if (promise)
				promise->fulfiller = nullptr;

			promise = other.promise;
			if (promise)
				promise->fulfiller = this;

			value = other.value;
			callback = std::move(other.callback);

			other.promise = nullptr;
			other.callback = nullptr;

			return *this;
		}
		~Fulfiller() {
			if (promise) {
				if (value)
					(new Fulfiller<void>(std::move(*this)))->allocated = true;
				else
					promise->fulfiller = nullptr;
			}
		}

		bool is_pending() const {
			if (value)
				return false;
			return callback != nullptr || promise != nullptr;
		}

		void fulfill() {
			value = true;
			if (callback) {
				callback();
				callback = nullptr;
			}
		}
	};

	template <typename T>
	class Promise
	{
		friend class PromiseFactory;
		friend class Fulfiller<T>;

		PromisePart<T> p;

	public:
		Promise() {
			p.fulfiller = nullptr;
		}
		Promise(Promise &&other) {
			p.fulfiller = nullptr;
			*this = std::move(other);
		}
		Promise& operator=(Promise &&other) {
			if (p.fulfiller) {
				p.fulfiller->promise = nullptr;
				p.fulfiller->callback = nullptr;
			}

			p.fulfiller = other.p.fulfiller;
			if (p.fulfiller)
				p.fulfiller->promise = &p;

			other.p.fulfiller = nullptr;

			return *this;
		}
		~Promise() {
			if (p.fulfiller)
				p.fulfiller->callback = nullptr;
			detach();
		}

		void detach() {
			if (p.fulfiller) {
				p.fulfiller->promise = nullptr;
				if (p.fulfiller->allocated)
					delete p.fulfiller;
			}
			p.fulfiller = nullptr;
		}

		bool is_pending() const {
			if (!p.fulfiller)
				return false;
			return !p.fulfiller->value;
		}

		Promise<T>& then(const typename CallbackType<T>::type &func) {
			if (!p.fulfiller)
				return *this;
			if (p.fulfiller->value) {
				if constexpr (std::is_void<T>::value)
					func();
				else
					func(std::move(*p.fulfiller->value));
			} else {
				p.fulfiller->callback = func;
			}
			return *this;
		}
	};

	class PromiseFactory
	{
		PromiseFactory() = delete;

	public:
		template <typename T>
		static std::pair<Promise<T>, Fulfiller<T>> create() {
			Promise<T> promise;
			Fulfiller<T> fulfiller;
			promise.p.fulfiller = &fulfiller;
			fulfiller.promise = &promise.p;
			return std::make_pair(std::move(promise), std::move(fulfiller));
		}
	};

	template <typename T>
	class FulfillerBufferedQueue
	{
		std::vector<Fulfiller<T>> fulfiller_queue;
		std::vector<T> data_queue;

	public:
		size_t pending_packets() {
			return data_queue.size();
		}

		size_t pending_fulfillers() {
			return fulfiller_queue.size();
		}

		void push_data(const T& packet) {
			if (!fulfiller_queue.empty()) {
				auto it = fulfiller_queue.begin();
				Fulfiller<T> f = std::move(*it);
				fulfiller_queue.erase(it);
				f.fulfill(packet);
			} else {
				data_queue.push_back(packet);
			}
		}

		void push_data(T&& packet) {
			if (!fulfiller_queue.empty()) {
				auto it = fulfiller_queue.begin();
				Fulfiller<T> f = std::move(*it);
				fulfiller_queue.erase(it);
				f.fulfill(std::move(packet));
			} else {
				data_queue.push_back(std::move(packet));
			}
		}

		void add(Fulfiller<T> &&f) {
			if (!data_queue.empty()) {
				auto it = data_queue.begin();
				T packet = std::move(*it);
				data_queue.erase(it);
				f.fulfill(std::move(packet));
			} else {
				fulfiller_queue.push_back(std::move(f));
			}
		}

		Promise<T>  create_and_add() {
			std::pair<Promise<T>, Fulfiller<T>> pair = PromiseFactory::create<T>();
			add(std::move(pair.second));
			return std::move(pair.first);
		}
	};

	template <typename T>
	class FulfillerList
	{
		std::vector<Fulfiller<T>> list;

	public:
		size_t pending() {
			return list.size();
		}

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
		size_t pending() {
			return list.size();
		}

		void add(Fulfiller<void> &&f) {
			list.push_back(std::move(f));
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
