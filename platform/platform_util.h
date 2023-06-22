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
	class DeferredFulfillment;

	template <typename T>
	class PromisePart
	{
		friend class PromiseFactory;
		friend class Promise<T>;
		friend class Fulfiller<T>;

		Fulfiller<T>* fulfiller;
	};

	class TypeErasedFulfiller
	{
		friend class DeferredFulfillment;

		virtual void execute_callback(bool defer) = 0;
	public:
		virtual ~TypeErasedFulfiller() = default;
	};

	class DeferredFulfillment
	{
		template <typename T> friend class Fulfiller;

	private:
		static std::vector<std::unique_ptr<TypeErasedFulfiller>> list;

	public:
		static bool execute() {
			if (list.empty())
				return false;

			auto tmp = std::move(list);
			list.clear();
			for (auto &f : tmp)
				f->execute_callback(false);

			return true;
		}
	};

	template <typename T>
	class Fulfiller final : public TypeErasedFulfiller
	{
		friend class PromiseFactory;
		friend class Promise<T>;

		PromisePart<T>* promise;
		std::optional<T> value;
		typename CallbackType<T>::type callback;
		bool unmanaged;

		virtual void execute_callback(bool defer) override {
			if (!callback)
				return;
			if (!defer) {
				callback(std::move(*value));
				if (promise)
					promise->fulfiller = nullptr;
				promise = nullptr;
				callback = nullptr;
				if (unmanaged)
					delete this;
			}
			else {
				if (unmanaged) {
					DeferredFulfillment::list.push_back(std::unique_ptr<Fulfiller<T>>(this));
					unmanaged = false;
				} else {
					DeferredFulfillment::list.push_back(std::make_unique<Fulfiller<T>>(std::move(*this)));
				}
			}
		}

	public:
		Fulfiller() {
			promise = nullptr;
			unmanaged = false;
		}
		Fulfiller(Fulfiller &&other) {
			promise = nullptr;
			unmanaged = false;
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
		virtual ~Fulfiller() override {
			if (promise) {
				if (value)
					(new Fulfiller<T>(std::move(*this)))->unmanaged = true;
				else
					promise->fulfiller = nullptr;
			}
		}

		bool is_pending() const {
			if (value)
				return false;
			return callback != nullptr || promise != nullptr;
		}

		void fulfill(T&& arg, bool defer = true) {
			value = std::move(arg);
			execute_callback(defer);
		}

		void fulfill(const T& arg, bool defer = true) {
			value = arg;
			execute_callback(defer);
		}
	};

	template <>
	class Fulfiller<void> final : public TypeErasedFulfiller
	{
		friend class PromiseFactory;
		friend class Promise<void>;

		PromisePart<void>* promise;
		bool value;
		typename CallbackType<void>::type callback;
		bool unmanaged;

		virtual void execute_callback(bool defer) override {
			if (!callback)
				return;
			if (!defer) {
				callback();
				if (promise)
					promise->fulfiller = nullptr;
				promise = nullptr;
				callback = nullptr;
				if (unmanaged)
					delete this;
			}
			else {
				if (unmanaged) {
					DeferredFulfillment::list.push_back(std::unique_ptr<Fulfiller<void>>(this));
					unmanaged = false;
				} else {
					DeferredFulfillment::list.push_back(std::make_unique<Fulfiller<void>>(std::move(*this)));
				}
			}
		}

	public:
		Fulfiller() {
			promise = nullptr;
			value = false;
			unmanaged = false;
		}
		Fulfiller(Fulfiller &&other) {
			promise = nullptr;
			unmanaged = false;
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
		virtual ~Fulfiller() override {
			if (promise) {
				if (value)
					(new Fulfiller<void>(std::move(*this)))->unmanaged = true;
				else
					promise->fulfiller = nullptr;
			}
		}

		bool is_pending() const {
			if (value)
				return false;
			return callback != nullptr || promise != nullptr;
		}

		void fulfill(bool defer = true) {
			value = true;
			execute_callback(defer);
		}
	};

	template <typename T>
	class Promise
	{
		friend class PromiseFactory;
		friend class Fulfiller<T>;

		PromisePart<T> p;

		void cancel() {
			if (p.fulfiller)
				p.fulfiller->callback = nullptr;
		}

	public:
		Promise() {
			p.fulfiller = nullptr;
		}
		Promise(Promise &&other) {
			p.fulfiller = nullptr;
			*this = std::move(other);
		}
		Promise& operator=(Promise &&other) {
			cancel();
			detach();

			p.fulfiller = other.p.fulfiller;
			if (p.fulfiller)
				p.fulfiller->promise = &p;

			other.p.fulfiller = nullptr;

			return *this;
		}
		~Promise() {
			cancel();
			detach();
		}

		void detach() {
			if (p.fulfiller) {
				p.fulfiller->promise = nullptr;
				if (p.fulfiller->unmanaged)
					delete p.fulfiller;
			}
			p.fulfiller = nullptr;
		}

		Promise<T>&& then(const typename CallbackType<T>::type &func) {
			if (p.fulfiller) {
				p.fulfiller->callback = func;
				if (p.fulfiller->value)
					p.fulfiller->execute_callback(true);
			}
			return std::move(*this);
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
		bool defer;

	public:
		FulfillerBufferedQueue(bool defer = true) : defer(defer) {}

		size_t pending_fulfillers() {
			fulfiller_queue.erase(std::remove_if(fulfiller_queue.begin(), fulfiller_queue.end(), [](const auto &entry) { return !entry.is_pending(); }), fulfiller_queue.end());
			return fulfiller_queue.size();
		}

		size_t pending_packets() {
			return data_queue.size();
		}

		void push_data(const T& packet) {
			auto it = fulfiller_queue.begin();
			while (it != fulfiller_queue.end() && !it->is_pending())
				++it;
			if (it != fulfiller_queue.end()) {
				Fulfiller<T> f = std::move(*it);
				fulfiller_queue.erase(it);
				f.fulfill(packet, defer);
			} else {
				data_queue.push_back(packet);
			}
		}

		void push_data(T&& packet) {
			auto it = fulfiller_queue.begin();
			while (it != fulfiller_queue.end() && !it->is_pending())
				++it;
			if (it != fulfiller_queue.end()) {
				Fulfiller<T> f = std::move(*it);
				fulfiller_queue.erase(it);
				f.fulfill(std::move(packet), defer);
			} else {
				data_queue.push_back(std::move(packet));
			}
		}

		void add(Fulfiller<T> &&f) {
			if (!f.is_pending())
				return;
			if (!data_queue.empty()) {
				auto it = data_queue.begin();
				T packet = std::move(*it);
				data_queue.erase(it);
				f.fulfill(std::move(packet), defer);
			} else {
				fulfiller_queue.push_back(std::move(f));
			}
		}

		Promise<T> create_and_add() {
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
			list.erase(std::remove_if(list.begin(), list.end(), [](const auto &entry) { return !entry.is_pending(); }), list.end());
			return list.size();
		}

		void add(Fulfiller<T> &&f) {
			list.push_back(std::move(f));
		}

		void fulfill_one(const T& arg, bool defer = true) {
			std::vector<Fulfiller<T>> tmp = std::move(list);
			list.clear();

			auto it = tmp.begin();
			while (it != tmp.end() && !it->is_pending())
				++it;
			if (it != tmp.end()) {
				tmp.begin()->fulfill(arg, defer);
				auto tmp_next = std::next(it);
				if (tmp_next != tmp.end())
					list.insert(list.begin(), std::make_move_iterator(tmp_next), std::make_move_iterator(tmp.end()));
			}
		}

		void fulfill_one(T&& arg, bool defer = true) {
			std::vector<Fulfiller<T>> tmp = std::move(list);
			list.clear();

			auto it = tmp.begin();
			while (it != tmp.end() && !it->is_pending())
				++it;
			if (it != tmp.end()) {
				tmp.begin()->fulfill(std::move(arg), defer);
				auto tmp_next = std::next(it);
				if (tmp_next != tmp.end())
					list.insert(list.begin(), std::make_move_iterator(tmp_next), std::make_move_iterator(tmp.end()));
			}
		}

		void fulfill_all(const T& arg, bool defer = true) {
			std::vector<Fulfiller<T>> tmp = std::move(list);
			list.clear();
			for (PlatformUtil::Fulfiller<T> &f : tmp)
				f.fulfill(arg, defer);
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
			list.erase(std::remove_if(list.begin(), list.end(), [](const auto &entry) { return !entry.is_pending(); }), list.end());
			return list.size();
		}

		void add(Fulfiller<void> &&f) {
			list.push_back(std::move(f));
		}

		void fulfill_one(bool defer = true) {
			std::vector<Fulfiller<void>> tmp = std::move(list);
			list.clear();

			auto it = tmp.begin();
			while (it != tmp.end() && !it->is_pending())
				++it;
			if (it != tmp.end()) {
				tmp.begin()->fulfill(defer);
				auto tmp_next = std::next(it);
				if (tmp_next != tmp.end())
					list.insert(list.begin(), std::make_move_iterator(tmp_next), std::make_move_iterator(tmp.end()));
			}
		}

		void fulfill_all(bool defer = true) {
			std::vector<Fulfiller<void>> tmp = std::move(list);
			list.clear();
			for (PlatformUtil::Fulfiller<void> &f : tmp)
				f.fulfill(defer);
		}

		Promise<void> create_and_add() {
			std::pair<Promise<void>, Fulfiller<void>> pair = PromiseFactory::create<void>();
			list.push_back(std::move(pair.second));
			return std::move(pair.first);
		}
	};
}
